// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/optimizer/editor.h"

#include "base/logging.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/validator.h"

namespace elang {
namespace optimizer {

namespace {
Control* ControlOf(Node* node) {
  if (auto const control = node->as<Control>())
    return control;
  for (auto user : node->users()) {
    if (auto const control = user->owner()->as<Control>())
      return control;
  }
  return nullptr;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : ErrorReporter(factory),
      FactoryUser(factory),
      control_(nullptr),
      function_(function) {
}

Editor::~Editor() {
}

Node* Editor::entry_node() const {
  return function_->entry_node();
}

Node* Editor::exit_node() const {
  return function_->exit_node();
}

void Editor::AppendInput(Node* node, Node* new_value) {
  node->AppendInput(new_value);
}

void Editor::ChangeInput(Node* node, size_t index, Node* new_value) {
  DCHECK(new_value->id() || new_value->IsLiteral()) << *new_value;
  DCHECK_NE(node, new_value);
  DCHECK_LE(new_value->id(), function_->max_node_id());
  node->InputAt(index)->SetValue(new_value);
}

void Editor::Commit() {
  DCHECK(control_);
  control_ = nullptr;
}

void Editor::Edit(Node* node) {
  DCHECK(!control_);
  control_ = ControlOf(node);
  DCHECK(control_) << *node;
}

Node* Editor::EmitParameter(size_t index) {
  DCHECK(control_);
  DCHECK_EQ(control_->input(0), entry_node());
  return NewParameter(entry_node(), index);
}

void Editor::ReplaceAllUses(Node* new_node, Node* old_node) {
  std::vector<Input*> users(old_node->users().begin(), old_node->users().end());
  for (auto const user : users)
    user->SetValue(new_node);
}

Control* Editor::SetBranch(Node* condition) {
  DCHECK(control_);
  return NewIf(control_, condition);
}

Control* Editor::SetJump(Control* target) {
  DCHECK(control_);
  DCHECK(target->IsValidControl());
  auto const jump_node = NewJump(control_);
  AppendInput(target, jump_node);
  return jump_node;
}

void Editor::SetPhiInput(EffectPhiNode* phi, Control* control, Effect* effect) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(effect->IsValidEffect()) << *effect;
  for (auto const input : phi->inputs()) {
    auto const phi_input = input->as<PhiOperandNode>();
    DCHECK(phi_input) << *phi << " has invalid operand " << *input;
    if (phi_input->input(0) == control) {
      static_cast<Node*>(phi_input)->InputAt(1)->SetValue(effect);
      return;
    }
  }
  phi->AppendInput(NewPhiOperand(control, effect));
}

void Editor::SetPhiInput(PhiNode* phi, Control* control, Node* value) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK_EQ(phi->output_type(), value->output_type()) << *phi << " " << *value;
  DCHECK(value->IsValidData()) << *value;
  DCHECK(!value->IsEffect()) << *value;
  for (auto const input : phi->inputs()) {
    auto const phi_input = input->as<PhiOperandNode>();
    DCHECK(phi_input) << *phi << " has invalid operand " << *input;
    if (phi_input->input(0) == control) {
      ChangeInput(phi_input, 1, value);
      return;
    }
  }
  phi->AppendInput(NewPhiOperand(control, value));
}

Control* Editor::SetRet(Effect* effect, Node* data) {
  DCHECK(control_);
  auto const merge_node = exit_node()->input(0)->as<MergeNode>();
  for (auto const predecessor : merge_node->inputs()) {
    auto const ret_node = predecessor->as<RetNode>();
    if (!ret_node)
      continue;
    if (ret_node->input(0) == control_) {
      ChangeInput(ret_node, 2, data);
      return ret_node;
    }
  }
  auto const new_ret_node = NewRet(control_, effect, data);
  merge_node->AppendInput(new_ret_node);
  return new_ret_node;
}

bool Editor::Validate() const {
  Validator validator(factory(), function());
  return validator.Validate();
}

}  // namespace optimizer
}  // namespace elang
