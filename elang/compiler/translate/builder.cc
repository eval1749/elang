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

//////////////////////////////////////////////////////////////////////
//
// Builder::BasicBlock
//
class Builder::BasicBlock final : public ZoneAllocated {
 public:
  BasicBlock(ir::Node* control, ir::Node* effect, const Variables& variables);
  ~BasicBlock() = delete;

  ir::Node* control() const { return control_; }
  ir::Node* effect() const { return effect_; }
  void set_effect(ir::Node* effect);
  const Variables& variables() const { return variables_; }

  void AssignVariable(sm::Variable* variable, ir::Node* value);
  void BindVariable(sm::Variable* variable, ir::Node* variable_value);
  void Commit();
  void UnbindVariable(sm::Variable* variable);
  ir::Node* VariableValueOf(sm::Variable* variable) const;

 private:
  bool committed_;

  // Control value associated to this |BasicBlock|.
  ir::Node* const control_;

  // Effect value at the end of this |BasicBlock|.
  ir::Node* effect_;

  Variables variables_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};

Builder::BasicBlock::BasicBlock(ir::Node* control,
                               ir::Node* effect,
                               const Variables& variables)
    : committed_(false),
      control_(control),
      effect_(effect),
      variables_(variables) {
  DCHECK(control_->IsValidControl());
  DCHECK(effect_->IsValidEffect());
}

void Builder::BasicBlock::set_effect(ir::Node* effect) {
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK_NE(effect_, effect) << *effect;
  DCHECK(!committed_);
  effect_ = effect;
}

void Builder::BasicBlock::AssignVariable(sm::Variable* variable,
                                        ir::Node* value) {
  DCHECK(!committed_);
  variables_[variable] = value;
}

void Builder::BasicBlock::BindVariable(sm::Variable* variable,
                                      ir::Node* variable_value) {
  DCHECK(!committed_);
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

void Builder::BasicBlock::Commit() {
  DCHECK(!committed_);
  committed_ = true;
}

void Builder::BasicBlock::UnbindVariable(sm::Variable* variable) {
  auto const it = variables_.find(variable);
  DCHECK(it != variables_.end()) << *variable;
  variables_.erase(it);
}

ir::Node* Builder::BasicBlock::VariableValueOf(sm::Variable* variable) const {
  auto const it = variables_.find(variable);
  DCHECK(it != variables_.end()) << *variable << " isn't resolved";
  DCHECK(it->second) << *variable << " has no value";
  return it->second;
}

//////////////////////////////////////////////////////////////////////
//
// Editor
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
  DCHECK(it != basic_blocks_.end());
  return it->second;
}

void Builder::BindVariable(sm::Variable* variable, ir::Node* value) {
  DCHECK(basic_block_);
  basic_block_->BindVariable(variable, value);
}

void Builder::EndBlock() {
  DCHECK(editor_->control());
  DCHECK_EQ(basic_block_, basic_blocks_[editor_->control()]);
  editor_->Commit();
  basic_block_->Commit();
  basic_block_ = nullptr;
}

ir::Node* Builder::EndBlockWithBranch(ir::Node* condition) {
  DCHECK(editor_->control());
  auto const if_node = editor_->SetBranch(condition);
  basic_blocks_[if_node] = basic_block_;
  EndBlock();
  return if_node;
}

void Builder::EndBlockWithJump(ir::Node* target) {
  if (!editor_->control())
    return;
  auto const node = editor_->SetJump(target);
  basic_blocks_[node] = basic_block_;
  EndBlock();
}

void Builder::EndBlockWithRet(ir::Node* data) {
  DCHECK(basic_block_);
  editor_->SetRet(basic_block_->effect(), data);
  EndBlock();
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

void Builder::StartIfBlock(ir::Node* control) {
  DCHECK(control->is<ir::IfTrueNode>() || control->is<ir::IfFalseNode>());
  DCHECK(!basic_block_);
  auto const if_node = control->input(0);
  auto const predecessor = BasicBlockOf(if_node->input(0));
  editor_->Edit(control);
  basic_block_ =
      NewBasicBlock(control, predecessor->effect(), predecessor->variables());
}

void Builder::StartMergeBlock(ir::Node* control) {
  DCHECK(control->is<ir::PhiOwnerNode>()) << *control;
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(!basic_block_);

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
  return basic_block_->VariableValueOf(variable);
}

}  // namespace compiler
}  // namespace elang
