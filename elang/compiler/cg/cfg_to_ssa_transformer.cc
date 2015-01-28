// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/cfg_to_ssa_transformer.h"

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
// RenameStackContainer
//
class RenameStackContainer final : public ZoneUser {
 public:
  explicit RenameStackContainer(Zone* zone);
  ~RenameStackContainer() = default;

  RenameStack* stack_for(hir::Value* value) const;

  void AssociatePhiToVariable(hir::PhiInstruction* phi, hir::Instruction* home);
  void DidEnterBlock(std::vector<RenameStack*>* kill_list);
  void DidExitBlock(std::vector<RenameStack*>* kill_list);
  void DidPush(RenameStack* rename_stack);
  void RegisterVariable(hir::Instruction* home);

 private:
  std::vector<RenameStack*>* kill_list_;
  std::unordered_map<hir::Instruction*, RenameStack*> map_;

  DISALLOW_COPY_AND_ASSIGN(RenameStackContainer);
};

RenameStackContainer::RenameStackContainer(Zone* zone)
    : ZoneUser(zone), kill_list_(nullptr) {
}

RenameStack* RenameStackContainer::stack_for(hir::Value* value) const {
  auto const home = value->as<hir::Instruction>();
  if (!home)
    return nullptr;
  auto const it = map_.find(home);
  return it == map_.end() ? nullptr : it->second;
}

void RenameStackContainer::AssociatePhiToVariable(hir::PhiInstruction* phi,
                                                  hir::Instruction* home) {
  DCHECK(!map_.count(phi));
  DCHECK(map_.count(home));
  map_[phi] = map_[home];
}

void RenameStackContainer::DidEnterBlock(std::vector<RenameStack*>* kill_list) {
  DCHECK(!kill_list_);
  kill_list_ = kill_list;
}

void RenameStackContainer::DidExitBlock(std::vector<RenameStack*>* kill_list) {
  DCHECK(kill_list_);
  for (auto const rename_stack : *kill_list)
    rename_stack->Pop();
  kill_list_ = nullptr;
}

void RenameStackContainer::DidPush(RenameStack* rename_stack) {
  kill_list_->push_back(rename_stack);
}

void RenameStackContainer::RegisterVariable(hir::Instruction* home) {
  DCHECK(!map_.count(home));
  map_[home] = new (zone()) RenameStack(zone());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaTransformer::Impl
//
class CfgToSsaTransformer::Impl final : public hir::InstructionVisitor,
                                        public ZoneOwner {
 public:
  Impl(hir::Factory* factory,
       hir::Function* function,
       const VariableUsages* variable_usages);
  ~Impl() final {}

  void Run();

 private:
  hir::Editor* editor() { return &editor_; }

  hir::Instruction* GetHomeFor(hir::PhiInstruction* phi) const;
  void InsertPhi(hir::BasicBlock* block, const VariableUsages::Data* data);
  void InsertPhis(const VariableUsages::Data* data);
  void RenameVariables(hir::BasicBlock* block);

  // hir::InstructionVisitor
  void VisitLoad(hir::LoadInstruction* instr) final;
  void VisitPhi(hir::PhiInstruction* instr) final;
  void VisitStore(hir::StoreInstruction* instr) final;

  hir::Editor editor_;
  hir::DominatorTree* const dominator_tree_;
  RenameStackContainer rename_tracker_;
  const VariableUsages* const variable_usages_;

  std::unordered_map<const VariableUsages*, RenameStack*> rename_map_;
  std::unordered_map<hir::Value*, const VariableUsages::Data*> home_map_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

CfgToSsaTransformer::Impl::Impl(hir::Factory* factory,
                                hir::Function* function,
                                const VariableUsages* variable_usages)
    : editor_(factory, function),
      dominator_tree_(hir::ComputeDominatorTree(zone(), function)),
      rename_tracker_(zone()),
      variable_usages_(variable_usages) {
}

hir::Instruction* CfgToSsaTransformer::Impl::GetHomeFor(
    hir::PhiInstruction* phi) const {
  auto const it = home_map_.find(phi);
  return it == home_map_.end() ? nullptr : it->second->home();
}

void CfgToSsaTransformer::Impl::InsertPhi(hir::BasicBlock* block,
                                          const VariableUsages::Data* data) {
  editor()->Edit(block);
  auto const phi = editor()->NewPhi(data->type());
  rename_tracker_.AssociatePhiToVariable(phi, data->home());
  editor()->Commit();
}

void CfgToSsaTransformer::Impl::InsertPhis(const VariableUsages::Data* data) {
  if (data->is_local())
    return;
  auto const home = data->home();
  rename_tracker_.RegisterVariable(home);

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

void CfgToSsaTransformer::Impl::RenameVariables(hir::BasicBlock* block) {
  std::vector<RenameStack*> kill_list;
  rename_tracker_.DidEnterBlock(&kill_list);
  editor()->Edit(block);
  for (auto const phi : block->phi_instructions())
    VisitPhi(phi);
  for (auto const instruction : block->instructions())
    instruction->Accept(this);
  editor()->Commit();
  // Update `phi` instructions in successors
  for (auto const successor : block->successors()) {
    editor()->Edit(successor);
    for (auto const phi : successor->phi_instructions()) {
      auto const rename_stack = rename_tracker_.stack_for(phi);
      if (!rename_stack)
        continue;
      editor()->SetPhiInput(phi, block, rename_stack->top());
    }
    editor()->Commit();
  }
  for (auto const child : dominator_tree_->node_of(block)->children())
    RenameVariables(child->value()->as<hir::BasicBlock>());
  rename_tracker_.DidExitBlock(&kill_list);
}

// The entry point of CFG to SSA transformer.
void CfgToSsaTransformer::Impl::Run() {
  // TODO(eval1749) If |function_| have exception handlers, we should analyze
  // liveness of variables.
  for (auto const variable_data :
       variable_usages_->local_variables_of(editor()->function())) {
    InsertPhis(variable_data);
  }
  RenameVariables(editor()->entry_block());
}

// hir::InstructionVisitor
void CfgToSsaTransformer::Impl::VisitLoad(hir::LoadInstruction* instr) {
  auto const rename_stack = rename_tracker_.stack_for(instr->input(0));
  if (!rename_stack)
    return;
  // Replace all uses of `load` instruction |instr| by top of rename stack.
  auto const value = rename_stack->top();
  for (auto const user : instr->users())
    user->SetValue(value);
  editor()->Edit(instr->basic_block());
  editor()->RemoveInstruction(instr);
  editor()->Commit();
}

void CfgToSsaTransformer::Impl::VisitPhi(hir::PhiInstruction* instr) {
  auto const rename_stack = rename_tracker_.stack_for(instr);
  if (!rename_stack)
    return;
  rename_stack->Push(instr);
}

void CfgToSsaTransformer::Impl::VisitStore(hir::StoreInstruction* instr) {
  auto const rename_stack = rename_tracker_.stack_for(instr->input(0));
  if (!rename_stack)
    return;
  rename_stack->Push(instr->input(1));
  editor()->Edit(instr->basic_block());
  editor()->RemoveInstruction(instr);
  editor()->Commit();
}

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaTransformer
//
CfgToSsaTransformer::CfgToSsaTransformer(hir::Factory* factory,
                                         hir::Function* function,
                                         const VariableUsages* usages)
    : impl_(new Impl(factory, function, usages)) {
}

CfgToSsaTransformer::~CfgToSsaTransformer() {
}

void CfgToSsaTransformer::Run() {
  impl_->Run();
}

}  // namespace compiler
}  // namespace elang
