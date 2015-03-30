// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/node_factory_user.h"

#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_factory.h"

namespace elang {
namespace optimizer {

NodeFactoryUser::NodeFactoryUser(NodeFactory* node_factory)
    : node_factory_(node_factory) {
}

NodeFactoryUser::~NodeFactoryUser() {
}

Node* NodeFactoryUser::false_value() const {
  return node_factory_->false_value();
}

Node* NodeFactoryUser::true_value() const {
  return node_factory_->true_value();
}

Node* NodeFactoryUser::void_value() const {
  return node_factory_->void_value();
}

Node* NodeFactoryUser::NewCall(Effect* effect, Node* callee, Node* arguments) {
  return node_factory_->NewCall(effect, callee, arguments);
}

Control* NodeFactoryUser::NewControlGet(Node* input, size_t field) {
  return node_factory_->NewControlGet(input, field);
}

Node* NodeFactoryUser::NewDynamicCast(Type* output_type, Node* input) {
  return node_factory_->NewDynamicCast(output_type, input);
}

Effect* NodeFactoryUser::NewEffectGet(Node* input, size_t field) {
  return node_factory_->NewEffectGet(input, field);
}

Effect* NodeFactoryUser::NewEffectPhi(PhiOwnerNode* owner) {
  return node_factory_->NewEffectPhi(owner);
}

Node* NodeFactoryUser::NewFloatCmp(FloatCondition condition,
                                   Node* left,
                                   Node* right) {
  return node_factory_->NewFloatCmp(condition, left, right);
}

Node* NodeFactoryUser::NewGet(Node* input, size_t field) {
  return node_factory_->NewGet(input, field);
}

Control* NodeFactoryUser::NewIf(Control* control, Node* data) {
  return node_factory_->NewIf(control, data);
}

Control* NodeFactoryUser::NewIfFalse(Control* control) {
  return node_factory_->NewIfFalse(control);
}

Control* NodeFactoryUser::NewIfTrue(Control* control) {
  return node_factory_->NewIfTrue(control);
}

Node* NodeFactoryUser::NewIntCmp(IntCondition condition,
                                 Node* left,
                                 Node* right) {
  return node_factory_->NewIntCmp(condition, left, right);
}

Control* NodeFactoryUser::NewJump(Control* control) {
  return node_factory_->NewJump(control);
}

PhiOwnerNode* NodeFactoryUser::NewMerge(const std::vector<Control*>& inputs) {
  return node_factory_->NewMerge(inputs);
}

Node* NodeFactoryUser::NewParameter(Node* input, size_t field) {
  return node_factory_->NewParameter(input, field);
}

Node* NodeFactoryUser::NewPhi(Type* output_type, PhiOwnerNode* owner) {
  return node_factory_->NewPhi(output_type, owner);
}

Node* NodeFactoryUser::NewPhiOperand(Control* control, Node* data) {
  return node_factory_->NewPhiOperand(control, data);
}

Node* NodeFactoryUser::NewReference(Type* type, AtomicString* name) {
  return node_factory_->NewReference(type, name);
}

Control* NodeFactoryUser::NewRet(Control* control, Effect* effect, Node* data) {
  return node_factory_->NewRet(control, effect, data);
}

Node* NodeFactoryUser::NewTuple(Type* output_type) {
  return node_factory_->NewTuple(output_type);
}

Node* NodeFactoryUser::NewStaticCast(Type* output_type, Node* input) {
  return node_factory_->NewStaticCast(output_type, input);
}

Node* NodeFactoryUser::NewTuple(const std::vector<Node*>& inputs) {
  return node_factory_->NewTuple(inputs);
}

// Arithmetic nodes
#define V(Name, ...)                                          \
  Node* NodeFactoryUser::New##Name(Node* left, Node* right) { \
    return node_factory_->New##Name(left, right);             \
  }
FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V)
V(IntShl)
V(IntShr)
#undef V

// Literal nodes
#define V(Name, mnemonic, data_type)                 \
  Node* NodeFactoryUser::New##Name(data_type data) { \
    return node_factory_->New##Name(data);           \
  }
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

}  // namespace optimizer
}  // namespace elang
