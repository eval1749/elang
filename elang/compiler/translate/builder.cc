// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/translate/builder.h"

#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/optimizer/editor.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace compiler {

namespace {

using VariableMap = std::unordered_map<sm::Variable*, ir::Data*>;

//////////////////////////////////////////////////////////////////////
//
// VariableScope represents lexical variable scope.
//
class VariableScope final {
 public:
  explicit VariableScope(VariableScope* outer);
  ~VariableScope();

  VariableScope* outer() const { return outer_; }
  const VariableMap& map() const { return map_; }
  const std::vector<sm::Variable*>& variables() const { return list_; }

  void Assign(sm::Variable* variable, ir::Data* data);
  void Bind(sm::Variable* variable, ir::Data* data);
  ir::Data* ValueOf(sm::Variable* variable) const;

 private:
  ir::Data* ValueFor(sm::Variable* variable) const;

  std::vector<sm::Variable*> list_;
  VariableMap map_;
  VariableScope* const outer_;

  DISALLOW_COPY_AND_ASSIGN(VariableScope);
};

VariableScope::VariableScope(VariableScope* outer) : outer_(outer) {
  if (!outer)
    return;
  list_.insert(list_.end(), outer->list_.begin(), outer->list_.end());
}

VariableScope::~VariableScope() {
}

void VariableScope::Assign(sm::Variable* variable, ir::Data* data) {
  for (auto runner = this; runner; runner = runner->outer()) {
    auto const it = runner->map_.find(variable);
    if (it != runner->map_.end()) {
      it->second = data;
      return;
    }
  }
  NOTREACHED() << *variable;
}

void VariableScope::Bind(sm::Variable* variable, ir::Data* data) {
  DCHECK(!ValueFor(variable));
  if (variable->storage() == sm::StorageClass::Void)
    return;
  DCHECK(variable->storage() == sm::StorageClass::Local ||
         variable->storage() == sm::StorageClass::ReadOnly);
  // TODO(eval1749) We should introduce |sm::StorageClass::Register| and
  // use here instead of |sm::StorageClass::Local|.
  map_.insert(std::make_pair(variable, data));
  if (variable->storage() == sm::StorageClass::ReadOnly)
    return;
  list_.push_back(variable);
}

ir::Data* VariableScope::ValueFor(sm::Variable* variable) const {
  for (auto runner = this; runner; runner = runner->outer()) {
    auto const it = runner->map_.find(variable);
    if (it != runner->map_.end())
      return it->second;
  }
  return nullptr;
}

ir::Data* VariableScope::ValueOf(sm::Variable* variable) const {
  auto const value = ValueFor(variable);
  DCHECK(value);
  return value;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Builder::BasicBlock
//
class Builder::BasicBlock final : public ZoneAllocated {
 public:
  BasicBlock(Zone* zone, ir::Control* control, ir::Effect* effect);
  ~BasicBlock() = delete;

  ir::Control* end_node() const;
  ir::Effect* effect() const { return effect_; }
  void set_effect(ir::Effect* effect);
  ir::Control* start_node() const { return start_node_; }
  const ZoneVector<sm::Variable*>& variables() const;

  void AddPhiVariable(sm::Variable* variable, ir::PhiNode* phi);
  void Commit(ir::Control* end_node, const VariableScope* var_map);
  bool HasPhiFor(sm::Variable* variable) const;
  sm::Variable* PhiVariableOf(ir::PhiNode* phi) const;
  ir::Data* ValueOf(sm::Variable* variable) const;

 private:
  // Effect value at the end of this |BasicBlock|.
  ir::Effect* effect_;
  ir::Control* end_node_;

  ZoneUnorderedMap<ir::PhiNode*, sm::Variable*> phi_map_;
  ZoneUnorderedMap<sm::Variable*, ir::PhiNode*> phi_var_map_;

  ir::Control* start_node_;

  // Variables at the end of this |BasicBlock|.
  ZoneVector<sm::Variable*> variables_;
  ZoneUnorderedMap<sm::Variable*, ir::Data*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};

Builder::BasicBlock::BasicBlock(Zone* zone,
                                ir::Control* start_node,
                                ir::Effect* effect)
    : effect_(effect),
      end_node_(nullptr),
      phi_map_(zone),
      phi_var_map_(zone),
      start_node_(start_node),
      variables_(zone),
      value_map_(zone) {
  DCHECK(start_node_->IsValidControl()) << *start_node_;
  DCHECK(start_node_->IsBlockStart()) << *start_node_;
  DCHECK(effect_->IsValidEffect()) << *effect_;
}

const ZoneVector<sm::Variable*>& Builder::BasicBlock::variables() const {
  DCHECK(end_node_);
  return variables_;
}

ir::Control* Builder::BasicBlock::end_node() const {
  DCHECK(end_node_);
  return end_node_;
}

void Builder::BasicBlock::set_effect(ir::Effect* effect) {
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK_NE(effect_, effect) << *effect;
  DCHECK(!end_node_);
  effect_ = effect;
}

void Builder::BasicBlock::AddPhiVariable(sm::Variable* variable,
                                         ir::PhiNode* phi) {
  DCHECK(!phi_map_.count(phi)) << *variable;
  DCHECK(!phi_var_map_.count(variable)) << *variable;
  phi_map_.insert(std::make_pair(phi, variable));
  phi_var_map_.insert(std::make_pair(variable, phi));
}

void Builder::BasicBlock::Commit(ir::Control* end_node,
                                 const VariableScope* var_scope) {
  DCHECK(!end_node_);
  DCHECK(end_node->IsBlockEnd()) << *end_node;
  DCHECK(value_map_.empty());
  DCHECK(variables_.empty());
  end_node_ = end_node;
  if (end_node->opcode() == ir::Opcode::Ret ||
      end_node->opcode() == ir::Opcode::Throw) {
    // Since |RetNode| and |ThrowNode| terminate normal control flow, we don't
    // need to merge variables in successor.
    return;
  }
  variables_.insert(variables_.end(), var_scope->variables().begin(),
                    var_scope->variables().end());
  for (auto const variable : variables_)
    value_map_.insert(std::make_pair(variable, var_scope->ValueOf(variable)));
}

bool Builder::BasicBlock::HasPhiFor(sm::Variable* variable) const {
  return phi_var_map_.count(variable) != 0;
}

sm::Variable* Builder::BasicBlock::PhiVariableOf(ir::PhiNode* phi) const {
  auto const it = phi_map_.find(phi);
  DCHECK(it != phi_map_.end());
  return it->second;
}

ir::Data* Builder::BasicBlock::ValueOf(sm::Variable* variable) const {
  DCHECK(end_node_);
  auto const it = value_map_.find(variable);
  DCHECK(it != value_map_.end());
  return it->second;
}

//////////////////////////////////////////////////////////////////////
//
// Builder::VariableTracker
//
class Builder::VariableTracker final {
 public:
  VariableTracker();
  ~VariableTracker();

  const VariableScope* scope() const { return scope_.get(); }

  void Assign(sm::Variable* variable, ir::Data* new_value);
  void Bind(sm::Variable* variable, ir::Data* value);
  void EndBlock();
  void EndScope();
  void StartBlock();
  void StartScope();
  ir::Data* ValueOf(sm::Variable* variable) const;

 private:
  VariableMap map_;
  std::unique_ptr<VariableScope> scope_;

  DISALLOW_COPY_AND_ASSIGN(VariableTracker);
};

Builder::VariableTracker::VariableTracker()
    : scope_(new VariableScope(nullptr)) {
}

Builder::VariableTracker::~VariableTracker() {
  while (scope_)
    scope_.reset(scope_->outer());
}

void Builder::VariableTracker::Assign(sm::Variable* variable,
                                      ir::Data* new_value) {
  if (!map_.count(variable))
    map_.insert(std::make_pair(variable, scope_->ValueOf(variable)));
  scope_->Assign(variable, new_value);
}

void Builder::VariableTracker::Bind(sm::Variable* variable, ir::Data* value) {
  scope_->Bind(variable, value);
}

// Restores values of variables modified in the block.
void Builder::VariableTracker::EndBlock() {
  for (auto var_val : map_)
    scope_->Assign(var_val.first, var_val.second);
  map_.clear();
}

void Builder::VariableTracker::EndScope() {
  for (auto var_val : scope_->map()) {
    auto const it = map_.find(var_val.first);
    if (it == map_.end())
      continue;
    map_.erase(it);
  }
  scope_.reset(scope_->outer());
}

void Builder::VariableTracker::StartBlock() {
  DCHECK(map_.empty());
}

void Builder::VariableTracker::StartScope() {
  scope_.reset(new VariableScope(scope_.release()));
}

ir::Data* Builder::VariableTracker::ValueOf(sm::Variable* variable) const {
  return scope_->ValueOf(variable);
}

//////////////////////////////////////////////////////////////////////
//
// Builder
//
Builder::Builder(ir::Factory* factory, ir::Function* function)
    : basic_block_(nullptr),
      editor_(new ir::Editor(factory, function)),
      variable_tracker_(new VariableTracker()) {
  auto const entry_node = function->entry_node();
  StartBlock(NewBasicBlock(entry_node, editor_->NewGetEffect(entry_node)));
}

Builder::~Builder() {
  DCHECK(!basic_block_);
  DCHECK(!editor_->control());
  DCHECK(editor_->Validate()) << editor_->errors();
}

void Builder::AssignVariable(sm::Variable* variable, ir::Data* value) {
  DCHECK(basic_block_);
  variable_tracker_->Assign(variable, value);
}

Builder::BasicBlock* Builder::BasicBlockOf(ir::Control* control) {
  auto const it = basic_blocks_.find(control);
  DCHECK(it != basic_blocks_.end()) << *control;
  return it->second;
}

void Builder::BindVariable(sm::Variable* variable, ir::Data* value) {
  DCHECK(basic_block_);
  variable_tracker_->Bind(variable, value);
}

ir::Data* Builder::Call(ir::Data* callee, ir::Node* arguments) {
  DCHECK(basic_block_) << *callee;
  auto const callee_type = callee->output_type()->as<ir::FunctionType>();
  DCHECK(callee_type) << *callee;
  auto const call = editor_->NewCall(editor_->control(), basic_block_->effect(),
                                     callee, arguments);
  basic_block_->set_effect(editor_->NewGetEffect(call));
  basic_blocks_[call] = basic_block_;
  editor_->Commit();
  editor_->Edit(call);
  if (callee_type->return_type()->is<ir::VoidType>())
    return editor_->void_value();
  return editor_->NewGetData(call);
}

void Builder::EndBlock(ir::Control* end_node) {
  DCHECK(end_node->IsBlockEnd());
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, basic_blocks_[editor_->control()]);
  DCHECK(!basic_blocks_.count(end_node));
  basic_blocks_.insert(std::make_pair(end_node, basic_block_));
  editor_->Commit();
  basic_block_->Commit(end_node, variable_tracker_->scope());
  variable_tracker_->EndBlock();
  basic_block_ = nullptr;
}

ir::Control* Builder::EndBlockWithBranch(ir::Data* condition) {
  DCHECK(basic_block_);
  auto const if_node = editor_->SetBranch(condition);
  EndBlock(if_node);
  return if_node;
}

ir::Control* Builder::EndBlockWithJump(ir::Control* target_node) {
  DCHECK(target_node->IsBlockStart());
  if (!basic_block_)
    return nullptr;
  auto const jump_node = editor_->SetJump(target_node);
  auto const basic_block = basic_block_;
  EndBlock(jump_node);
  PopulatePhiNodesIfNeeded(target_node, basic_block);
  return jump_node;
}

void Builder::EndBlockWithRet(ir::Data* data) {
  DCHECK(basic_block_);
  auto const ret_node = editor_->SetRet(basic_block_->effect(), data);
  EndBlock(ret_node);
}

void Builder::EndLoopBlock(ir::Data* condition,
                           ir::Control* true_target_node,
                           ir::Control* false_target_node) {
  DCHECK(false_target_node->is<ir::PhiOwnerNode>()) << *false_target_node;
  DCHECK(true_target_node->is<ir::PhiOwnerNode>()) << *true_target_node;
  DCHECK(basic_block_);

  auto const if_node = EndBlockWithBranch(condition);

  StartIfBlock(editor_->NewIfTrue(if_node));
  EndBlockWithJump(true_target_node);

  StartIfBlock(editor_->NewIfFalse(if_node));
  EndBlockWithJump(false_target_node);
}

void Builder::EndVariableScope() {
  variable_tracker_->EndScope();
}

Builder::BasicBlock* Builder::NewBasicBlock(ir::Control* start_node,
                                            ir::Effect* effect) {
  DCHECK(start_node->IsBlockStart()) << *start_node;
  DCHECK(start_node->IsValidControl()) << *start_node;
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK(!basic_blocks_.count(start_node)) << *start_node;
  auto const zone = ZoneOwner::zone();
  auto const basic_block = new (zone) BasicBlock(zone, start_node, effect);
  basic_blocks_.insert(std::make_pair(start_node, basic_block));
  return basic_block;
}

ir::Data* Builder::NewLoad(ir::Data* base_pointer, ir::Data* pointer) {
  DCHECK(basic_block_);
  return editor_->NewLoad(basic_block_->effect(), base_pointer, pointer);
}

ir::PhiOwnerNode* Builder::NewMergeBlock() {
  return editor_->NewMerge({});
}

ir::Data* Builder::ParameterAt(size_t index) {
  return editor_->ParameterAt(index);
}

void Builder::PopulatePhiNodesIfNeeded(ir::Control* control,
                                       const BasicBlock* predecessor) {
  auto const phi_owner = control->as<ir::PhiOwnerNode>();
  if (!phi_owner)
    return;
  auto const it = basic_blocks_.find(phi_owner);
  if (it == basic_blocks_.end())
    return;
  auto const end_node = predecessor->end_node();
  if (auto const effect_phi = phi_owner->effect_phi())
    editor_->SetPhiInput(effect_phi, end_node, predecessor->effect());
  auto const phi_block = it->second;
  for (auto const phi : phi_owner->phi_nodes()) {
    auto const variable = phi_block->PhiVariableOf(phi);
    auto const value = predecessor->ValueOf(variable);
    editor_->SetPhiInput(phi, end_node, value);
  }
}

void Builder::StartBlock(BasicBlock* block) {
  DCHECK(!basic_block_);
  editor_->Edit(block->start_node());
  basic_block_ = block;
  variable_tracker_->StartBlock();
}

// Start loop block and populate variable table with phi nodes.
ir::Control* Builder::StartDoLoop(ir::LoopNode* loop_block) {
  DCHECK(basic_block_);
  auto const jump_node = editor_->SetJump(loop_block);
  auto const predecessor = basic_block_;
  EndBlock(jump_node);

  // We assume all variables are changed in the loop.
  auto const effect_phi = editor_->NewEffectPhi(loop_block);
  editor_->SetPhiInput(effect_phi, jump_node, predecessor->effect());

  StartBlock(NewBasicBlock(loop_block, effect_phi));
  for (auto variable : predecessor->variables()) {
    auto const value = predecessor->ValueOf(variable);
    auto const phi = editor_->NewPhi(value->output_type(), loop_block);
    basic_block_->AddPhiVariable(variable, phi);
    AssignVariable(variable, phi);
    editor_->SetPhiInput(phi, jump_node, value);
  }

  return loop_block;
}

void Builder::StartIfBlock(ir::Control* control) {
  DCHECK(control->is<ir::IfTrueNode>() || control->is<ir::IfFalseNode>());
  DCHECK(!basic_block_);
  auto const if_node = control->input(0);
  auto const predecessor = BasicBlockOf(if_node->control(0));
  StartBlock(NewBasicBlock(control, predecessor->effect()));
  for (auto const variable : predecessor->variables())
    AssignVariable(variable, predecessor->ValueOf(variable));
}

void Builder::StartMergeBlock(ir::PhiOwnerNode* phi_owner) {
  DCHECK(phi_owner->IsValidControl()) << *phi_owner;
  DCHECK(phi_owner->CountInputs());
  DCHECK(!basic_block_);

  auto effect_phi = phi_owner->effect_phi();
  auto effect = static_cast<ir::Effect*>(effect_phi);

  if (!effect) {
    for (auto const input : phi_owner->inputs()) {
      auto const predecessor = BasicBlockOf(input->as<ir::Control>());
      if (!effect) {
        effect = predecessor->effect();
      } else if (effect != predecessor->effect()) {
        effect_phi = editor_->NewEffectPhi(phi_owner);
        effect = effect_phi;
        break;
      }
    }
  }

  StartBlock(NewBasicBlock(phi_owner, effect));

  VariableMap var_map;

  // Figure out effect and variables in |control|.
  for (auto const input : phi_owner->inputs()) {
    auto const predecessor = BasicBlockOf(input->as<ir::Control>());
    for (auto const variable : predecessor->variables()) {
      if (basic_block_->HasPhiFor(variable))
        continue;
      auto const value = predecessor->ValueOf(variable);
      auto const it = var_map.find(variable);
      if (it == var_map.end()) {
        var_map.insert(std::make_pair(variable, value));
        AssignVariable(variable, value);
        continue;
      }
      if (it->second == value)
        continue;
      auto const phi = editor_->NewPhi(value->output_type(), phi_owner);
      basic_block_->AddPhiVariable(variable, phi);
      AssignVariable(variable, phi);
    }
  }

  if (!effect_phi && phi_owner->phi_nodes().empty())
    return;

  // Populate 'phi` operands.
  for (auto const input : phi_owner->inputs()) {
    auto const control = input->as<ir::Control>();
    auto const predecessor = BasicBlockOf(control);

    if (effect_phi)
      editor_->SetPhiInput(effect_phi, control, predecessor->effect());

    for (auto const phi : phi_owner->phi_nodes()) {
      auto const variable = basic_block_->PhiVariableOf(phi);
      editor_->SetPhiInput(phi, control, predecessor->ValueOf(variable));
    }
  }
}

void Builder::StartWhileLoop(ir::Data* condition,
                             ir::LoopNode* loop_block,
                             ir::PhiOwnerNode* break_block) {
  if (condition == editor_->true_value()) {
    StartDoLoop(loop_block);
    return;
  }
  if (condition == editor_->false_value()) {
    EndBlockWithJump(break_block);
    return;
  }
  auto const if_node = EndBlockWithBranch(condition);

  StartIfBlock(editor_->NewIfFalse(if_node));
  EndBlockWithJump(break_block);

  StartIfBlock(editor_->NewIfTrue(if_node));
  StartDoLoop(loop_block);
}

void Builder::StartVariableScope() {
  variable_tracker_->StartScope();
}

ir::Data* Builder::VariableValueOf(sm::Variable* variable) const {
  DCHECK(basic_block_) << *variable;
  return variable_tracker_->ValueOf(variable);
}

}  // namespace compiler
}  // namespace elang
