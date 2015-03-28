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
Node* ControlOf(Node* node) {
  if (node->IsControl())
    return node;
  for (auto user : node->users()) {
    if (user->owner()->IsControl())
      return user->owner();
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
  auto const variadic = node->as<VariadicNode>();
  variadic->AppendInput(new_value);
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

Node* Editor::SetBranch(Node* condition) {
  DCHECK(control_);
  auto const if_node = NewIf(control_, condition);
  auto const if_true = NewIfTrue(if_node);
  auto const if_false = NewIfFalse(if_node);
  auto const merge = NewMerge({if_true, if_false});
  return merge;
}

void Editor::SetJump(Node* target) {
  DCHECK(control_);
  DCHECK(target->IsValidControl());
  auto const jump_node = NewJump(control_);
  AppendInput(target, jump_node);
}

void Editor::SetPhiInput(Node* node, Node* control, Node* value) {
  auto const phi = node->as<PhiNode>();
  DCHECK(phi) << *node;
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(value->IsValidData()) << *value;
  for (auto const input : node->inputs()) {
    auto const phi_input = input->as<PhiOperandNode>();
    DCHECK(phi_input) << *phi << " has invalid operand " << *input;
    if (phi_input->input(0) == control) {
      ChangeInput(phi_input, 1, value);
      return;
    }
  }
  phi->AppendInput(NewPhiOperand(control, value));
}

void Editor::SetRet(Node* effect, Node* data) {
  DCHECK(control_);
  auto const merge_node = exit_node()->input(0)->as<MergeNode>();
  for (auto const predecessor : merge_node->inputs()) {
    auto const ret_node = predecessor->as<RetNode>();
    if (!ret_node)
      continue;
    if (ret_node->input(0) == control_) {
      ChangeInput(ret_node, 2, data);
      return;
    }
  }
  merge_node->AppendInput(NewRet(control_, effect, data));
}

bool Editor::Validate() const {
  Validator validator(factory(), function());
  return validator.Validate();
}

}  // namespace optimizer
}  // namespace elang
