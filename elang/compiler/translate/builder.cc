// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <unordered_set>

#include "elang/compiler/translate/builder.h"

#include "elang/compiler/semantics/nodes.h"
#include "elang/optimizer/editor.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace compiler {

typedef std::unordered_map<ir::PhiNode*, sm::Variable*> PhiMap;

//////////////////////////////////////////////////////////////////////
//
// Builder::BasicBlock
//
class Builder::BasicBlock final : public ZoneAllocated {
 public:
  BasicBlock(ir::Node* control, ir::Node* effect, const Variables& variables);
  ~BasicBlock() = delete;

  ir::Node* end_node() const;
  ir::Node* effect() const { return effect_; }
  void set_effect(ir::Node* effect);
  const PhiMap& phi_map() const { return phi_map_; }
  ir::Node* start_node() const { return start_node_; }
  const Variables& variables() const { return variables_; }

  void AssignVariable(sm::Variable* variable, ir::Node* value);
  void BindVariable(sm::Variable* variable, ir::Node* variable_value);
  void Commit(ir::Node* end_node);
  void PopulatePhiNodes(const BasicBlock* predecessor);
  void UnbindVariable(sm::Variable* variable);
  ir::Node* ValueOf(sm::Variable* variable) const;

 private:
  // Effect value at the end of this |BasicBlock|.
  ir::Node* effect_;
  ir::Node* end_node_;

  // A mapping from |sm::Variable| to |ir::PhiNode| at start of block.
  PhiMap phi_map_;

  ir::Node* start_node_;

  // A mapping from |sm::Variable| to |ir::Node| at end of block.
  Variables variables_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};

Builder::BasicBlock::BasicBlock(ir::Node* start_node,
                                ir::Node* effect,
                                const Variables& variables)
    : effect_(effect),
      end_node_(nullptr),
      start_node_(start_node),
      variables_(variables) {
  DCHECK(start_node_->IsValidControl());
  DCHECK(effect_->IsValidEffect());
  if (!start_node->is<ir::PhiOwnerNode>())
    return;
  for (auto const var_val : variables_) {
    auto const phi = var_val.second->as<ir::PhiNode>();
    if (!phi || phi->owner() != start_node)
      continue;
    phi_map_[phi] = var_val.first;
  }
}

ir::Node* Builder::BasicBlock::end_node() const {
  DCHECK(end_node_);
  return end_node_;
}

void Builder::BasicBlock::set_effect(ir::Node* effect) {
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK_NE(effect_, effect) << *effect;
  DCHECK(!end_node_);
  effect_ = effect;
}

void Builder::BasicBlock::AssignVariable(sm::Variable* variable,
                                         ir::Node* value) {
  DCHECK(!end_node_);
  variables_[variable] = value;
}

void Builder::BasicBlock::BindVariable(sm::Variable* variable,
                                       ir::Node* variable_value) {
  DCHECK(!end_node_);
  if (variable->storage() == sm::StorageClass::Void)
    return;
  DCHECK(!variables_.count(variable));
  if (variable->storage() == sm::StorageClass::ReadOnly) {
    variables_[variable] = variable_value;
    return;
  }

  // TODO(eval1749) We should introduce |sm::StorageClass::Register| and
  // use here instead of |sm::StorageClass::Local|.
  DCHECK_EQ(variable->storage(), sm::StorageClass::Local);
  variables_[variable] = variable_value;
}

void Builder::BasicBlock::Commit(ir::Node* end_node) {
  DCHECK(!end_node_);
  DCHECK_NE(start_node_, end_node) << "start=" << *start_node_
                                   << " end=" << *end_node;
  end_node_ = end_node;
}

void Builder::BasicBlock::UnbindVariable(sm::Variable* variable) {
  auto const it = variables_.find(variable);
  DCHECK(it != variables_.end()) << *variable;
  variables_.erase(it);
}

ir::Node* Builder::BasicBlock::ValueOf(sm::Variable* variable) const {
  auto const it = variables_.find(variable);
  DCHECK(it != variables_.end()) << *variable << " isn't in " << *start_node_;
  DCHECK(it->second) << *variable << " has no value in " << *start_node_;
  return it->second;
}

//////////////////////////////////////////////////////////////////////
//
// Builder
//
Builder::Builder(ir::Factory* factory, ir::Function* function)
    : basic_block_(nullptr), editor_(new ir::Editor(factory, function)) {
  auto const entry_node = function->entry_node();
  auto const control = editor_->NewGet(entry_node, 0);
  editor_->Edit(control);
  basic_block_ = NewBasicBlock(control, editor_->NewGet(entry_node, 1), {});
}

Builder::~Builder() {
  DCHECK(!basic_block_);
  DCHECK(!editor_->control());
  DCHECK(editor_->Validate()) << editor_->errors();
}

ir::Node* Builder::control() const {
  return editor_->control();
}

void Builder::AssignVariable(sm::Variable* variable, ir::Node* value) {
  DCHECK(basic_block_);
  basic_block_->AssignVariable(variable, value);
}

Builder::BasicBlock* Builder::BasicBlockOf(ir::Node* control) {
  auto const it = basic_blocks_.find(control);
  DCHECK(it != basic_blocks_.end()) << *control;
  return it->second;
}

void Builder::BindVariable(sm::Variable* variable, ir::Node* value) {
  DCHECK(basic_block_);
  basic_block_->BindVariable(variable, value);
}

void Builder::EndBlock(ir::Node* control) {
  DCHECK(basic_block_);
  DCHECK_EQ(basic_block_, basic_blocks_[editor_->control()]);
  basic_blocks_[control] = basic_block_;
  editor_->Commit();
  basic_block_->Commit(control);
  basic_block_ = nullptr;
}

ir::Node* Builder::EndBlockWithBranch(ir::Node* condition) {
  DCHECK(basic_block_);
  auto const if_node = editor_->SetBranch(condition);
  EndBlock(if_node);
  return if_node;
}

void Builder::EndBlockWithJump(ir::Node* target_node) {
  if (!basic_block_)
    return;
  auto const jump_node = editor_->SetJump(target_node);
  auto const basic_block = basic_block_;
  EndBlock(jump_node);
  PopulatePhiNodesIfNeeded(target_node, basic_block);
}

void Builder::EndBlockWithRet(ir::Node* data) {
  DCHECK(basic_block_);
  auto const ret_node = editor_->SetRet(basic_block_->effect(), data);
  EndBlock(ret_node);
}

void Builder::EndLoopBlock(ir::Node* condition,
                           ir::Node* true_target_node,
                           ir::Node* false_target_node) {
  DCHECK(false_target_node->is<ir::PhiOwnerNode>()) << *false_target_node;
  DCHECK(true_target_node->is<ir::PhiOwnerNode>()) << *true_target_node;
  DCHECK(basic_block_);

  auto const if_node = EndBlockWithBranch(condition);

  StartIfBlock(editor_->NewIfTrue(if_node));
  EndBlockWithJump(true_target_node);

  StartIfBlock(editor_->NewIfFalse(if_node));
  EndBlockWithJump(false_target_node);
}

Builder::BasicBlock* Builder::NewBasicBlock(ir::Node* control,
                                            ir::Node* effect,
                                            const Variables& variables) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK(!basic_blocks_.count(control));
  auto const basic_block =
      new (ZoneOwner::zone()) BasicBlock(control, effect, variables);
  basic_blocks_[control] = basic_block;
  return basic_block;
}

ir::Node* Builder::ParameterAt(size_t index) {
  return editor_->EmitParameter(index);
}

void Builder::PopulatePhiNodesIfNeeded(ir::Node* control,
                                       const BasicBlock* predecessor) {
  auto const phi_owner = control->as<ir::PhiOwnerNode>();
  if (!phi_owner)
    return;
  auto const it = basic_blocks_.find(phi_owner);
  if (it == basic_blocks_.end())
    return;
  auto const phi_block = it->second;
  for (auto const phi : phi_owner->phi_nodes()) {
    if (phi->output_type()->is<ir::EffectType>()) {
      editor_->SetPhiInput(phi, predecessor->end_node(), predecessor->effect());
      continue;
    }
    auto const phi_var = phi_block->phi_map().find(phi);
    DCHECK(phi_var != phi_block->phi_map().end());
    auto const value = predecessor->ValueOf(phi_var->second);
    editor_->SetPhiInput(phi, predecessor->end_node(), value);
  }
}

void Builder::StartIfBlock(ir::Node* control) {
  DCHECK(control->is<ir::IfTrueNode>() || control->is<ir::IfFalseNode>());
  DCHECK(!basic_block_);
  auto const if_node = control->input(0);
  auto const predecessor = BasicBlockOf(if_node->input(0));
  editor_->Edit(control);
  basic_block_ =
      NewBasicBlock(control, predecessor->effect(), predecessor->variables());
}

// Start loop block and populate variable table with phi nodes.
ir::Node* Builder::StartLoopBlock() {
  DCHECK(basic_block_);
  auto const loop_control = editor_->NewMerge({});
  auto const jump_node = editor_->SetJump(loop_control);
  auto const predecessor = basic_block_;
  EndBlock(jump_node);

  editor_->Edit(loop_control);

  // We assume all variables are changed in the loop.
  auto const effect_phi = editor_->NewPhi(editor_->effect_type(), loop_control);
  editor_->SetPhiInput(effect_phi, jump_node, predecessor->effect());

  Variables variables;
  for (auto var_val : predecessor->variables()) {
    auto const phi =
        editor_->NewPhi(var_val.second->output_type(), loop_control);
    variables[var_val.first] = phi;
    editor_->SetPhiInput(phi, jump_node, var_val.second);
  }

  basic_block_ = NewBasicBlock(loop_control, effect_phi, variables);
  return loop_control;
}

void Builder::StartMergeBlock(ir::Node* control) {
  DCHECK(control->is<ir::PhiOwnerNode>()) << *control;
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(!basic_block_);

  if (!control->CountInputs())
    return;

  auto effect = static_cast<ir::Node*>(nullptr);
  auto effect_phi = static_cast<ir::Node*>(nullptr);
  Variables variables;
  std::unordered_map<sm::Variable*, ir::Node*> variable_phis;

  // Figure out effect and variables in |control|.
  for (auto const input : control->inputs()) {
    auto const predecessor = BasicBlockOf(input);

    if (!effect_phi) {
      if (!effect) {
        effect = predecessor->effect();
      } else if (effect != predecessor->effect()) {
        effect_phi = editor_->NewPhi(editor_->effect_type(), control);
        effect = effect_phi;
      }
    }

    for (auto const var_value : predecessor->variables()) {
      auto const variable = var_value.first;
      if (variable_phis.count(variable))
        continue;
      auto const value = var_value.second;
      auto const it = variables.find(variable);
      if (it == variables.end()) {
        variables[variable] = value;
        continue;
      }
      if (it->second == value)
        continue;
      auto const phi = editor_->NewPhi(value->output_type(), control);
      variables[variable] = phi;
      variable_phis[variable] = phi;
    }
  }

  editor_->Edit(control);
  basic_block_ = NewBasicBlock(control, effect, variables);

  if (!effect_phi && variable_phis.empty())
    return;

  // Populate 'phi` operands.
  for (auto const input : control->inputs()) {
    auto const predecessor = BasicBlockOf(input);

    if (effect_phi)
      editor_->SetPhiInput(effect_phi, input, predecessor->effect());

    for (auto const var_phi : variable_phis) {
      auto const it = predecessor->variables().find(var_phi.first);
      DCHECK(it != predecessor->variables().end()) << *var_phi.first;
      editor_->SetPhiInput(var_phi.second, input, it->second);
    }
  }
}

void Builder::UnbindVariable(sm::Variable* variable) {
  DCHECK(basic_block_);
  basic_block_->UnbindVariable(variable);
}

ir::Node* Builder::VariableValueOf(sm::Variable* variable) const {
  DCHECK(basic_block_);
  return basic_block_->ValueOf(variable);
}

}  // namespace compiler
}  // namespace elang
