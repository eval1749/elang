// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/cfg_to_ssa_converter.h"

#include <unordered_set>
#include <vector>

#include "elang/base/zone_user.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/cg/variable_usages.h"
#include "elang/hir/analysis/dominator_tree_builder.h"
#include "elang/hir/editor.h"
#include "elang/hir/instructions.h"
#include "elang/hir/instruction_visitor.h"
#include "elang/hir/values.h"

namespace elang {
namespace compiler {

namespace {

//////////////////////////////////////////////////////////////////////
//
// RenameStack
//
class RenameStack final : public ZoneAllocated {
 public:
  explicit RenameStack(Zone* zone) : stack_(zone) {}

  hir::Value* top() const { return stack_.back(); }
  void Pop() { stack_.pop_back(); }
  void Push(hir::Value* value) { stack_.push_back(value); }

 private:
  ~RenameStack() = delete;

  ZoneVector<hir::Value*> stack_;

  DISALLOW_COPY_AND_ASSIGN(RenameStack);
};

//////////////////////////////////////////////////////////////////////
//
// Renamer
//
class Renamer final : public ZoneUser, public hir::InstructionVisitor {
 public:
  Renamer(Zone* zone,
          hir::Editor* editor,
          const hir::DominatorTree* dominator_tree);
  ~Renamer() final = default;

  void RegisterPhi(hir::PhiInstruction* phi, hir::Instruction* home);
  void RegisterVariable(hir::Instruction* home);
  void Run();

 private:
  class RenameScope {
   public:
    explicit RenameScope(Renamer* passlet);
    ~RenameScope();

   private:
    std::vector<RenameStack*> kill_list_;
    Renamer* const renamer_;

    DISALLOW_COPY_AND_ASSIGN(RenameScope);
  };

  hir::Editor* editor() const { return editor_; }
  RenameStack* stack_for(hir::Value* value) const;

  void Push(RenameStack* rename_stack, hir::Value* new_value);
  void RenameVariables(hir::BasicBlock* block);

  // hir::InstructionVisitor
  void VisitLoad(hir::LoadInstruction* instr) final;
  void VisitPhi(hir::PhiInstruction* instr) final;
  void VisitStore(hir::StoreInstruction* instr) final;

  const hir::DominatorTree* dominator_tree_;
  hir::Editor* const editor_;
  std::vector<RenameStack*>* kill_list_;
  std::unordered_map<hir::Instruction*, RenameStack*> map_;

  DISALLOW_COPY_AND_ASSIGN(Renamer);
};

Renamer::RenameScope::RenameScope(Renamer* renamer) : renamer_(renamer) {
  renamer_->kill_list_ = &kill_list_;
}

Renamer::RenameScope::~RenameScope() {
  renamer_->kill_list_ = nullptr;
  for (auto const rename_stack : kill_list_)
    rename_stack->Pop();
}

Renamer::Renamer(Zone* zone,
                 hir::Editor* editor,
                 const hir::DominatorTree* dominator_tree)
    : ZoneUser(zone),
      dominator_tree_(dominator_tree),
      editor_(editor),
      kill_list_(nullptr) {
}

RenameStack* Renamer::stack_for(hir::Value* value) const {
  auto const home = value->as<hir::Instruction>();
  if (!home)
    return nullptr;
  auto const it = map_.find(home);
  return it == map_.end() ? nullptr : it->second;
}

void Renamer::Push(RenameStack* rename_stack, hir::Value* new_value) {
  rename_stack->Push(new_value);
  kill_list_->push_back(rename_stack);
}

void Renamer::RegisterPhi(hir::PhiInstruction* phi, hir::Instruction* home) {
  DCHECK(!map_.count(phi));
  DCHECK(map_.count(home));
  map_[phi] = map_[home];
}

void Renamer::RegisterVariable(hir::Instruction* home) {
  DCHECK(!map_.count(home));
  map_[home] = new (zone()) RenameStack(zone());
}

void Renamer::RenameVariables(hir::BasicBlock* block) {
  RenameScope rename_scope(this);

  {
    hir::Editor::ScopedEdit edit_scope(editor(), block);
    for (auto const phi : block->phi_instructions())
      VisitPhi(phi);

    // Since |instruction| can be removed during visiting, we advance iterator
    // before calling visiting function.
    auto& instructions = block->instructions();
    for (auto it = instructions.begin(); it != instructions.end();) {
      auto const instruction = *it;
      ++it;
      instruction->Accept(this);
    }
  }

  // Update `phi` instructions in successors
  for (auto const successor : block->successors()) {
    hir::Editor::ScopedEdit edit_scope(editor(), successor);
    for (auto const phi : successor->phi_instructions()) {
      auto const rename_stack = stack_for(phi);
      if (!rename_stack)
        continue;
      editor()->SetPhiInput(phi, block, rename_stack->top());
    }
  }

  for (auto const child : dominator_tree_->node_of(block)->children())
    RenameVariables(child->value()->as<hir::BasicBlock>());
}

// The entry point of |Renamer|.
void Renamer::Run() {
  RenameVariables(editor()->entry_block());
}

// hir::InstructionVisitor
void Renamer::VisitLoad(hir::LoadInstruction* instr) {
  auto const rename_stack = stack_for(instr->input(0));
  if (!rename_stack)
    return;
  editor()->ReplaceAll(rename_stack->top(), instr);
}

void Renamer::VisitPhi(hir::PhiInstruction* instr) {
  auto const rename_stack = stack_for(instr);
  if (!rename_stack)
    return;
  Push(rename_stack, instr);
}

void Renamer::VisitStore(hir::StoreInstruction* instr) {
  auto const rename_stack = stack_for(instr->input(0));
  if (!rename_stack)
    return;
  Push(rename_stack, instr->input(1));
  editor()->RemoveInstruction(instr);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaConverter::Impl
//
class CfgToSsaConverter::Impl final : public ZoneOwner {
 public:
  Impl(hir::Factory* factory,
       hir::Function* function,
       const VariableUsages* variable_usages);
  ~Impl() = default;

  void Run();

 private:
  hir::Editor* editor() { return &editor_; }

  hir::Instruction* GetHomeFor(hir::PhiInstruction* phi) const;
  void InsertPhi(hir::BasicBlock* block, const VariableUsages::Data* data);
  void InsertPhis(const VariableUsages::Data* data);

  hir::Editor editor_;
  hir::DominatorTree* const dominator_tree_;
  Renamer renamer_;
  const VariableUsages* const variable_usages_;

  std::unordered_map<const VariableUsages*, RenameStack*> rename_map_;
  std::unordered_map<hir::Value*, const VariableUsages::Data*> home_map_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

CfgToSsaConverter::Impl::Impl(hir::Factory* factory,
                              hir::Function* function,
                              const VariableUsages* variable_usages)
    : editor_(factory, function),
      dominator_tree_(hir::ComputeDominatorTree(zone(), function)),
      renamer_(zone(), &editor_, dominator_tree_),
      variable_usages_(variable_usages) {
}

hir::Instruction* CfgToSsaConverter::Impl::GetHomeFor(
    hir::PhiInstruction* phi) const {
  auto const it = home_map_.find(phi);
  return it == home_map_.end() ? nullptr : it->second->home();
}

void CfgToSsaConverter::Impl::InsertPhi(hir::BasicBlock* block,
                                        const VariableUsages::Data* data) {
  editor()->Edit(block);
  auto const phi = editor()->NewPhi(data->type());
  renamer_.RegisterPhi(phi, data->home());
  editor()->Commit();
}

void CfgToSsaConverter::Impl::InsertPhis(const VariableUsages::Data* data) {
  auto const home = data->home();
  renamer_.RegisterVariable(home);
  if (data->is_local())
    return;

  std::unordered_set<hir::BasicBlock*> work_set;

  // Initialize work list
  for (auto const frontier :
       dominator_tree_->node_of(editor()->entry_block())->frontiers()) {
    work_set.insert(frontier->value()->as<hir::BasicBlock>());
  }

  // Add all variable change to work list
  for (auto const user : home->users()) {
    if (!user->instruction()->is<hir::StoreInstruction>())
      continue;
    auto const using_block = user->instruction()->basic_block();
    if (work_set.count(using_block))
      continue;
    for (auto const frontier :
         dominator_tree_->node_of(using_block)->frontiers()) {
      work_set.insert(frontier->value()->as<hir::BasicBlock>());
    }
  }

  std::vector<hir::BasicBlock*> work_list(work_set.begin(), work_set.end());
  while (!work_list.empty()) {
    auto const block = work_list.back();
    work_list.pop_back();
    InsertPhi(block, data);
    for (auto const frontier : dominator_tree_->node_of(block)->frontiers()) {
      auto const frontier_block = frontier->value()->as<hir::BasicBlock>();
      if (work_set.count(frontier_block))
        continue;
      work_set.insert(frontier_block);
      work_list.push_back(frontier_block);
    }
  }
}

// The entry point of CFG to SSA transformer.
void CfgToSsaConverter::Impl::Run() {
  // TODO(eval1749) If |function_| have exception handlers, we should analyze
  // liveness of variables.
  for (auto const variable_data :
       variable_usages_->local_variables_of(editor()->function())) {
    InsertPhis(variable_data);
  }

  // Rename variables
  renamer_.Run();

  // Remove variable home maker instructions.
  for (auto const variable_data :
       variable_usages_->local_variables_of(editor()->function())) {
    auto const home = variable_data->home();
    auto const home_block = home->basic_block();
    if (!editor()->basic_block()) {
      editor()->Edit(home_block);
    } else if (editor()->basic_block() != home_block) {
      editor()->Commit();
      editor()->Edit(home_block);
    }
    editor()->RemoveInstruction(home);
  }
  if (!editor()->basic_block())
    return;
  editor()->Commit();
}

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaConverter
//
CfgToSsaConverter::CfgToSsaConverter(hir::Factory* factory,
                                     hir::Function* function,
                                     const VariableUsages* usages)
    : impl_(new Impl(factory, function, usages)) {
}

CfgToSsaConverter::~CfgToSsaConverter() {
}

void CfgToSsaConverter::Run() {
  impl_->Run();
}

}  // namespace compiler
}  // namespace elang
