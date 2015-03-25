// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/editor.h"

#include "base/logging.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : ErrorReporter(factory),
      FactoryUser(factory),
      effect_node_(function->entry_node()),
      function_(function) {
  control_stack_.push(entry_node());
}

Editor::~Editor() {
}

Node* Editor::entry_node() const {
  return function_->entry_node();
}

Node* Editor::exit_node() const {
  return function_->exit_node();
}

Node* Editor::EmitParameter(size_t index) {
  return NewParameter(function_->entry_node(), index);
}

void Editor::EndIf() {
  auto const control = PopControl();
  DCHECK(control->is<IfNode>()) << *control;
}

void Editor::EndWithRet(Node* data) {
  auto const control = PopControl();
  for (auto const input : exit_node()->inputs()) {
    auto const ret_node = input->as<RetNode>();
    if (!ret_node)
      continue;
    if (ret_node->input(0) == control) {
      SetInput(ret_node, 1, data);
      return;
    }
  }
  exit_node()->as<ExitNode>()->AppendInput(NewRet(control, data));
}

Node* Editor::PopControl() {
  DCHECK(!control_stack_.empty());
  auto const control = control_stack_.top();
  control_stack_.pop();
  return control;
}

void Editor::SetInput(Node* node, size_t index, Node* new_value) {
  DCHECK(new_value->IsValidData()) << *new_value;
  DCHECK_NE(node, new_value);
  DCHECK_LE(new_value->id(), function_->max_node_id());
  node->InputAt(index)->SetValue(new_value);
}

void Editor::StartIf(Node* data) {
  control_stack_.push(NewIf(PopControl(), data));
}

void Editor::StartElse() {
  auto const control = control_stack_.top();
  DCHECK(control->is<IfNode>()) << *control;
  control_stack_.push(NewIfFalse(control));
}

void Editor::StartThen() {
  auto const control = control_stack_.top();
  DCHECK(control->is<IfNode>()) << *control;
  control_stack_.push(NewIfTrue(control));
}

}  // namespace optimizer
}  // namespace elang
