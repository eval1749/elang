// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>
#include <vector>

#include "elang/optimizer/editor.h"

#include "base/logging.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/formatters/text_formatter.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/validator.h"

namespace elang {
namespace optimizer {

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

EntryNode* Editor::entry_node() const {
  return function_->entry_node();
}

ExitNode* Editor::exit_node() const {
  return function_->exit_node();
}

void Editor::AppendInput(Node* node, Node* new_value) {
  node->AppendInput(new_value);
}

void Editor::ChangeInput(Node* node, size_t index, Node* new_value) {
  DCHECK(new_value->id() || new_value->IsLiteral()) << *new_value;
  DCHECK_NE(node, new_value);
  DCHECK_LE(new_value->id(), function_->max_node_id());
  node->InputAt(index)->SetTo(new_value);
}

void Editor::Commit() {
  DCHECK(control_);
  control_ = nullptr;
}

void Editor::Discard(Node* node) {
  DCHECK(!node->IsUsed());
  auto const num_inputs = node->CountInputs();
  for (size_t position = 0; position < num_inputs; ++position)
    node->InputAt(position)->Reset();
}

void Editor::Edit(Control* control) {
  DCHECK(!control_);
  DCHECK(control);
  control_ = control;
}

Data* Editor::ParameterAt(size_t index) {
  DCHECK(control_);
  DCHECK_EQ(control_, entry_node()) << *control_;
  return NewParameter(entry_node(), index);
}

void Editor::RemoveControlInput(PhiOwnerNode* node, Control* control) {
  auto const it = std::find_if(node->inputs_.begin(), node->inputs_.end(),
                               [control](const InputHolder* holder) {
                                 return holder->input()->value() == control;
                               });
  DCHECK(it != node->inputs_.end()) << *node << " " << *control;
  (*it)->input()->Reset();
  node->inputs_.erase(it);
  if (auto const effect_phi = node->effect_phi())
    RemovePhiInput(effect_phi, control);
  for (auto const phi : node->phi_nodes())
    RemovePhiInput(phi, control);
}

void Editor::RemovePhiInput(EffectPhiNode* phi, Control* control) {
  auto& phi_inputs = phi->phi_inputs_;
  auto const it = std::find_if(phi_inputs.begin(), phi_inputs.end(),
                               [control](const PhiInputHolder* holder) {
                                 return holder->control() == control;
                               });
  DCHECK(it != phi_inputs.end());
  (*it)->input()->Reset();
  phi_inputs.erase(it);
}

void Editor::RemovePhiInput(PhiNode* phi, Control* control) {
  auto& phi_inputs = phi->phi_inputs_;
  auto const it = std::find_if(phi_inputs.begin(), phi_inputs.end(),
                               [control](const PhiInputHolder* holder) {
                                 return holder->control() == control;
                               });
  DCHECK(it != phi_inputs.end());
  (*it)->input()->Reset();
  phi_inputs.erase(it);
}

void Editor::ReplaceAllUses(Node* new_node, Node* old_node) {
  std::vector<UseEdge*> edges(old_node->use_edges().begin(),
                              old_node->use_edges().end());
  for (auto const edge : edges)
    edge->SetTo(new_node);
}

Control* Editor::SetBranch(Data* condition) {
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
  for (auto const phi_input : phi->phi_inputs()) {
    if (phi_input->control() == control) {
      phi_input->input()->SetTo(effect);
      return;
    }
  }
  auto const phi_input = new (zone()) PhiInputHolder(control);
  phi->phi_inputs_.push_back(phi_input);
  phi_input->input()->Init(phi, effect);
}

void Editor::SetPhiInput(PhiNode* phi, Control* control, Data* value) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK_EQ(phi->output_type(), value->output_type()) << *phi << " " << *value;
  DCHECK(value->IsValidData()) << *value;
  DCHECK(!value->IsEffect()) << *value;
  for (auto const phi_input : phi->phi_inputs()) {
    if (phi_input->control() == control) {
      phi_input->input()->SetTo(value);
      return;
    }
  }
  auto const phi_input = new (zone()) PhiInputHolder(control);
  phi->phi_inputs_.push_back(phi_input);
  phi_input->input()->Init(phi, value);
}

Control* Editor::SetRet(Effect* effect, Data* data) {
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

std::ostream& operator<<(std::ostream& ostream, const Editor& editor) {
  ostream << AsReversePostOrder(editor.function());
  if (editor.errors().empty())
    return ostream;
  return ostream << std::endl
                 << "Errors:" << std::endl
                 << editor.errors();
}

}  // namespace optimizer
}  // namespace elang
