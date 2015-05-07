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

Data* NodeFactoryUser::false_value() const {
  return node_factory_->false_value();
}

Data* NodeFactoryUser::true_value() const {
  return node_factory_->true_value();
}

Data* NodeFactoryUser::void_value() const {
  return node_factory_->void_value();
}

Control* NodeFactoryUser::NewCall(Control* control,
                                  Effect* effect,
                                  Data* callee,
                                  Node* arguments) {
  return node_factory_->NewCall(control, effect, callee, arguments);
}

Data* NodeFactoryUser::NewDynamicCast(Type* output_type, Data* input) {
  return node_factory_->NewDynamicCast(output_type, input);
}

EffectPhiNode* NodeFactoryUser::NewEffectPhi(PhiOwnerNode* owner) {
  return node_factory_->NewEffectPhi(owner);
}

Data* NodeFactoryUser::NewElement(Data* array, Node* indexes) {
  return node_factory_->NewElement(array, indexes);
}

Data* NodeFactoryUser::NewFloatCmp(FloatCondition condition,
                                   Data* left,
                                   Data* right) {
  return node_factory_->NewFloatCmp(condition, left, right);
}

Data* NodeFactoryUser::NewGet(Tuple* input, size_t field) {
  return node_factory_->NewGet(input, field);
}

Data* NodeFactoryUser::NewGetData(Control* input) {
  return node_factory_->NewGetData(input);
}

Effect* NodeFactoryUser::NewGetEffect(Control* input) {
  return node_factory_->NewGetEffect(input);
}

Tuple* NodeFactoryUser::NewGetTuple(Control* input) {
  return node_factory_->NewGetTuple(input);
}

Control* NodeFactoryUser::NewIf(Control* control, Data* data) {
  return node_factory_->NewIf(control, data);
}

Control* NodeFactoryUser::NewIfFalse(Control* control) {
  return node_factory_->NewIfFalse(control);
}

Control* NodeFactoryUser::NewIfTrue(Control* control) {
  return node_factory_->NewIfTrue(control);
}

Data* NodeFactoryUser::NewIntCmp(IntCondition condition,
                                 Data* left,
                                 Data* right) {
  return node_factory_->NewIntCmp(condition, left, right);
}

Control* NodeFactoryUser::NewJump(Control* control) {
  return node_factory_->NewJump(control);
}

Data* NodeFactoryUser::NewLength(Data* array, size_t rank) {
  return node_factory_->NewLength(array, rank);
}

Data* NodeFactoryUser::NewLoad(Effect* effect,
                               Data* base_pointer,
                               Data* pointer) {
  return node_factory_->NewLoad(effect, base_pointer, pointer);
}

LoopNode* NodeFactoryUser::NewLoop() {
  return node_factory_->NewLoop();
}

PhiOwnerNode* NodeFactoryUser::NewMerge(const std::vector<Control*>& inputs) {
  return node_factory_->NewMerge(inputs);
}

Data* NodeFactoryUser::NewParameter(EntryNode* input, size_t field) {
  return node_factory_->NewParameter(input, field);
}

PhiNode* NodeFactoryUser::NewPhi(Type* output_type, PhiOwnerNode* owner) {
  return node_factory_->NewPhi(output_type, owner);
}

Data* NodeFactoryUser::NewReference(Type* type, AtomicString* name) {
  return node_factory_->NewReference(type, name);
}

Control* NodeFactoryUser::NewRet(Control* control, Effect* effect, Data* data) {
  return node_factory_->NewRet(control, effect, data);
}

Data* NodeFactoryUser::NewSizeOf(Type* type) {
  return node_factory_->NewSizeOf(type);
}

Data* NodeFactoryUser::NewStaticCast(Type* output_type, Data* input) {
  return node_factory_->NewStaticCast(output_type, input);
}

Effect* NodeFactoryUser::NewStore(Effect* effect,
                                  Data* anchor,
                                  Data* pointer,
                                  Data* new_value) {
  return node_factory_->NewStore(effect, anchor, pointer, new_value);
}

Tuple* NodeFactoryUser::NewTuple(const std::vector<Node*>& inputs) {
  return node_factory_->NewTuple(inputs);
}

// Arithmetic nodes
#define V(Name, ...)                                          \
  Data* NodeFactoryUser::New##Name(Data* left, Data* right) { \
    return node_factory_->New##Name(left, right);             \
  }
FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V)
V(IntShl)
V(IntShr)
#undef V

// Literal nodes
#define V(Name, mnemonic, data_type)                 \
  Data* NodeFactoryUser::New##Name(data_type data) { \
    return node_factory_->New##Name(data);           \
  }
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

}  // namespace optimizer
}  // namespace elang
