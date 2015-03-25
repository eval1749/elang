// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/validator.h"

#include "base/logging.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/error_code.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_visitor.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Validator::Context
//
class Validator::Context : public NodeVisitor {
 public:
  explicit Context(Validator* validator);
  ~Context() = default;

  bool is_valid() const { return is_valid_; }

 private:
  void Error(ErrorCode error_code, Node* node, Thing* detail);
  void Error(ErrorCode error_code, Node* node);
  void ErrorInInput(Node* node, int index);

  // NodeVisitor member functions
  void DoDefaultVisit(Node* node) final;
  void VisitEntry(EntryNode* node) final;
  void VisitExit(ExitNode* node) final;
  void VisitRet(RetNode* node) final;

  bool is_valid_;
  Validator* const validator_;

  DISALLOW_COPY_AND_ASSIGN(Context);
};

Validator::Context::Context(Validator* validator)
    : is_valid_(true), validator_(validator) {
}

void Validator::Context::Error(ErrorCode error_code,
                               Node* node,
                               Thing* detail) {
  validator_->Error(error_code, node, detail);
  is_valid_ = false;
}

void Validator::Context::Error(ErrorCode error_code, Node* node) {
  validator_->Error(error_code, node);
  is_valid_ = false;
}

void Validator::Context::ErrorInInput(Node* node, int index) {
  Error(ErrorCode::ValidateNodeInvalidInput, node, validator_->NewInt32(index));
}

void Validator::Context::DoDefaultVisit(Node* node) {
}

void Validator::Context::VisitEntry(EntryNode* node) {
  auto const tuple_type = node->output_type()->as<TupleType>();
  if (!tuple_type || tuple_type->size() != 3) {
    Error(ErrorCode::ValidateEntryNodeInvalidOutput, node);
    return;
  }
  if (!tuple_type->get(0)->is<ControlType>())
    Error(ErrorCode::ValidateEntryNodeNoControlOutput, node);
  if (!tuple_type->get(1)->is<EffectType>())
    Error(ErrorCode::ValidateEntryNodeNoEffectOutput, node);
  if (node->users().empty())
    Error(ErrorCode::ValidateEntryNodeNoUsers, node);
}

void Validator::Context::VisitExit(ExitNode* node) {
  bool has_control = false;
  bool has_effect = false;
  auto index = -1;
  for (auto const input : node->inputs()) {
    ++index;
    if (input->IsControl() && input->IsEffect()) {
      has_control = true;
      has_effect = true;
      continue;
    }
    if (input->IsControl()) {
      has_control = true;
      continue;
    }
    if (input->IsEffect()) {
      has_effect = true;
      continue;
    }
    ErrorInInput(node, index);
  }
  if (!has_control)
    Error(ErrorCode::ValidateExitNodeNoControlInput, node);
  if (!has_effect)
    Error(ErrorCode::ValidateExitNodeNoEffectInput, node);
}

void Validator::Context::VisitRet(RetNode* node) {
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
  if (!node->input(0)->IsValidEffect())
    ErrorInInput(node, 1);
}

//////////////////////////////////////////////////////////////////////
//
// Validator
//
Validator::Validator(Factory* factory, const Function* function)
    : ErrorReporter(factory), factory_(factory), function_(function) {
}

Validator::~Validator() {
}

Node* Validator::NewInt32(int data) {
  return factory_->NewInt32(data);
}

bool Validator::Validate(Node* node) {
  Context context(this);
  node->Accept(&context);
  return context.is_valid();
}

bool Validator::Validate() {
  DepthFirstTraversal<OnInputEdge, const Function> walker;
  Context context(this);
  walker.Traverse(function_, &context);
  return context.is_valid();
}

}  // namespace optimizer
}  // namespace elang
