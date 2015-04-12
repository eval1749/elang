// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "elang/optimizer/validator.h"

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
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

  void ValidatePhiInputs(Node* node,
                         PhiOwnerNode* owner,
                         const ZoneDeque<PhiInputHolder*>& phi_inputs);

  // NodeVisitor member functions
  void DoDefaultVisit(Node* node) final;
  void VisitCall(CallNode* node) final;
  void VisitEffectPhi(EffectPhiNode* node) final;
  void VisitElement(ElementNode* node) final;
  void VisitEntry(EntryNode* node) final;
  void VisitExit(ExitNode* node) final;
  void VisitGet(GetNode* node) final;
  void VisitGetData(GetDataNode* node) final;
  void VisitGetEffect(GetEffectNode* node) final;
  void VisitGetTuple(GetTupleNode* node) final;
  void VisitIf(IfNode* node) final;
  void VisitIfFalse(IfFalseNode* node) final;
  void VisitIfTrue(IfTrueNode* node) final;
  void VisitIntCmp(IntCmpNode* node) final;
  void VisitLength(LengthNode* node) final;
  void VisitLoad(LoadNode* node) final;
  void VisitPhi(PhiNode* node) final;
  void VisitRet(RetNode* node) final;
  void VisitParameter(ParameterNode* node) final;

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

void Validator::Context::ValidatePhiInputs(
    Node* node,
    PhiOwnerNode* owner,
    const ZoneDeque<PhiInputHolder*>& phi_inputs) {
  std::unordered_set<Node*> predecessors;
  for (auto predecessor : owner->inputs())
    predecessors.insert(predecessor);

  std::unordered_set<Node*> controls;
  auto index = 0;
  for (auto const phi_input : phi_inputs) {
    auto const input = phi_input->value();
    if (input->output_type() != node->output_type())
      ErrorInInput(node, index);
    if (!input->IsValidEffect() && !input->IsValidData())
      ErrorInInput(node, index);
    auto const control = phi_input->control();
    if (controls.count(control))
      ErrorInInput(node, index);
    if (!predecessors.count(control))
      ErrorInInput(node, index);
    controls.insert(control);
    ++index;
  }

  for (auto predecessor : owner->inputs()) {
    if (controls.count(predecessor))
      continue;
    Error(ErrorCode::ValidatePhiNodeMissing, node, predecessor);
  }
}

// NodeVisitor functions
void Validator::Context::DoDefaultVisit(Node* node) {
}

void Validator::Context::VisitCall(CallNode* node) {
  auto const output_type = node->output_type()->as<ControlType>();
  if (!output_type)
    return Error(ErrorCode::ValidateNodeInvalidOutput, node);

  if (!node->input(0)->output_type()->is<ControlType>())
    ErrorInInput(node, 0);

  if (!node->input(1)->output_type()->is<EffectType>())
    ErrorInInput(node, 1);

  auto const callee_type = node->input(2)->output_type()->as<FunctionType>();
  if (!callee_type)
    return ErrorInInput(node, 2);
  if (output_type->data_type() != callee_type->return_type())
    ErrorInInput(node, 2);

  if (node->input(3)->output_type() != callee_type->parameters_type())
    ErrorInInput(node, 3);
}

void Validator::Context::VisitEffectPhi(EffectPhiNode* node) {
  if (!node->owner()->IsValidControl())
    return Error(ErrorCode::ValidatePhiNodeInvalidOwner, node, node->owner());
  ValidatePhiInputs(node, node->owner(), node->phi_inputs());
}

void Validator::Context::VisitElement(ElementNode* node) {
  auto const array_pointer_type =
      node->input(0)->output_type()->as<PointerType>();
  if (!array_pointer_type)
    return ErrorInInput(node, 0);
  auto const array_type = array_pointer_type->pointee()->as<ArrayType>();
  if (!array_type)
    return ErrorInInput(node, 0);

  auto const pointer_type = node->output_type()->as<PointerType>();
  if (pointer_type && (pointer_type->pointee() != array_type->element_type()))
    return Error(ErrorCode::ValidateNodeInvalidOutput, node);

  if (array_type->rank() == 1) {
    if (!node->input(1)->output_type()->is<Int32Type>())
      ErrorInInput(node, 1);
    return;
  }

  auto const indexes_type = node->input(1)->output_type()->as<TupleType>();
  if (!indexes_type || array_type->rank() != indexes_type->size())
    return ErrorInInput(node, 1);
  for (auto const type : indexes_type->components()) {
    if (!type->is<Int32Type>())
      return ErrorInInput(node, 1);
  }
}

void Validator::Context::VisitEntry(EntryNode* node) {
  auto const output_type = node->output_type()->as<ControlType>();
  if (!output_type)
    Error(ErrorCode::ValidateNodeInvalidOutput, node);
  if (node->use_edges().empty())
    Error(ErrorCode::ValidateEntryNodeNoUsers, node);
}

void Validator::Context::VisitExit(ExitNode* node) {
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
}

void Validator::Context::VisitGet(GetNode* node) {
  auto const tuple_type = node->input(0)->output_type()->as<TupleType>();
  if (!tuple_type)
    return ErrorInInput(node, 0);
  if (node->field() >= tuple_type->size())
    return ErrorInInput(node, 0);
}

void Validator::Context::VisitGetData(GetDataNode* node) {
  auto const output_type = node->output_type();
  if (output_type->is_void())
    Error(ErrorCode::ValidateNodeInvalidOutput, node);
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
  auto const data_type =
      node->input(0)->output_type()->as<ControlType>()->data_type();
  if (data_type != output_type)
    ErrorInInput(node, 0);
}

void Validator::Context::VisitGetEffect(GetEffectNode* node) {
  if (!node->output_type()->is<EffectType>())
    Error(ErrorCode::ValidateNodeInvalidOutput, node);
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
  if (!node->input(0)->IsControlEffect())
    ErrorInInput(node, 0);
}

void Validator::Context::VisitGetTuple(GetTupleNode* node) {
  auto const output_type = node->output_type();
  if (!output_type->is<TupleType>())
    Error(ErrorCode::ValidateNodeInvalidOutput, node);
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
  auto const data_type =
      node->input(0)->output_type()->as<ControlType>()->data_type();
  if (data_type != output_type)
    ErrorInInput(node, 0);
}

void Validator::Context::VisitIf(IfNode* node) {
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
  if (!node->input(1)->IsValidData())
    ErrorInInput(node, 1);
  if (!node->input(1)->output_type()->is<BoolType>())
    ErrorInInput(node, 1);
}

void Validator::Context::VisitIfFalse(IfFalseNode* node) {
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
  if (!node->input(0)->is<IfNode>())
    ErrorInInput(node, 0);
}

void Validator::Context::VisitIfTrue(IfTrueNode* node) {
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
  if (!node->input(0)->is<IfNode>())
    ErrorInInput(node, 0);
}

void Validator::Context::VisitIntCmp(IntCmpNode* node) {
  if (!node->output_type()->is<BoolType>())
    Error(ErrorCode::ValidateNodeInvalidOutput, node);
  auto const operand_type = node->input(0)->output_type();
  if (operand_type != node->input(1)->output_type())
    ErrorInInput(node, 1);
  if (!operand_type->is_integer() && !operand_type->is<PointerType>())
    ErrorInInput(node, 0);

  auto const condition = node->condition();
  if (operand_type->is_signed()) {
    if (condition == IntCondition::UnsignedGreaterThan ||
        condition == IntCondition::UnsignedGreaterThanOrEqual ||
        condition == IntCondition::UnsignedLessThan ||
        condition == IntCondition::UnsignedLessThanOrEqual) {
      ErrorInInput(node, 0);
    }
  } else {
    if (condition == IntCondition::SignedGreaterThan ||
        condition == IntCondition::SignedGreaterThanOrEqual ||
        condition == IntCondition::SignedLessThan ||
        condition == IntCondition::SignedLessThanOrEqual) {
      ErrorInInput(node, 0);
    }
  }
}

void Validator::Context::VisitLength(LengthNode* node) {
  auto const pointer_type = node->input(0)->output_type()->as<PointerType>();
  if (!pointer_type)
    return ErrorInInput(node, 0);
  auto const array_type = pointer_type->pointee()->as<ArrayType>();
  if (!array_type)
    return ErrorInInput(node, 0);

  auto const rank_node = node->input(1)->as<Int32Node>();
  if (!rank_node)
    return ErrorInInput(node, 1);
  if (rank_node->data() >= base::checked_cast<int>(array_type->rank()))
    return ErrorInInput(node, 1);
}

void Validator::Context::VisitLoad(LoadNode* node) {
  if (!node->input(0)->IsValidEffect())
    ErrorInInput(node, 0);
  if (!node->input(1)->IsValidData())
    ErrorInInput(node, 1);
  if (!node->input(1)->output_type()->is<PointerType>())
    ErrorInInput(node, 1);
  if (!node->input(2)->IsValidData())
    ErrorInInput(node, 2);
  auto const pointer_type = node->input(2)->output_type()->as<PointerType>();
  if (!pointer_type || node->output_type() != pointer_type->pointee())
    ErrorInInput(node, 1);
}

void Validator::Context::VisitParameter(ParameterNode* node) {
  auto const entry_node = node->input(0)->as<EntryNode>();
  if (!entry_node) {
    ErrorInInput(node, 0);
    return;
  }
  if (node->output_type() != entry_node->parameter_type(node->field())) {
    Error(ErrorCode::ValidateNodeInvalidOutput, node);
    return;
  }
}

void Validator::Context::VisitPhi(PhiNode* node) {
  if (!node->owner()->IsValidControl())
    return Error(ErrorCode::ValidatePhiNodeInvalidOwner, node, node->owner());
  ValidatePhiInputs(node, node->owner(), node->phi_inputs());
}

void Validator::Context::VisitRet(RetNode* node) {
  if (!node->input(0)->IsValidControl())
    ErrorInInput(node, 0);
  if (!node->input(1)->IsValidEffect())
    ErrorInInput(node, 1);
  if (!node->input(2)->IsValidData())
    ErrorInInput(node, 2);
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
