// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/optimizer/editor.h"

#include "base/logging.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"

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

Node* EffectOf(Node* node) {
  if (node->IsEffect())
    return node;
  for (auto user : node->users()) {
    if (user->owner()->IsEffect())
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
  effect_ = EffectOf(node);
  if (effect_)
    return;
  effect_ = EffectOf(entry_node());
  DCHECK(effect_);
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
  auto const merge = NewMerge(if_true, if_false);
  return merge;
}

void Editor::SetRet(Node* data) {
  DCHECK(control_);
  DCHECK(effect_);
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
  merge_node->AppendInput(NewRet(control_, effect_, data));
}

}  // namespace optimizer
}  // namespace elang
