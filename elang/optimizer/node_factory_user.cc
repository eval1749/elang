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

Node* NodeFactoryUser::NewCall(Node* effect, Node* callee, Node* arguments) {
  return node_factory_->NewCall(effect, callee, arguments);
}

Node* NodeFactoryUser::NewGet(Node* input, size_t field) {
  return node_factory_->NewGet(input, field);
}

Node* NodeFactoryUser::NewIf(Node* control, Node* data) {
  return node_factory_->NewIf(control, data);
}

Node* NodeFactoryUser::NewIfFalse(Node* control) {
  return node_factory_->NewIfFalse(control);
}

Node* NodeFactoryUser::NewIfTrue(Node* control) {
  return node_factory_->NewIfTrue(control);
}

Node* NodeFactoryUser::NewMerge(Node* control0, Node* control1) {
  return node_factory_->NewMerge(control0, control1);
}

Node* NodeFactoryUser::NewParameter(Node* input, size_t field) {
  return node_factory_->NewParameter(input, field);
}

Node* NodeFactoryUser::NewReference(Type* type, AtomicString* name) {
  return node_factory_->NewReference(type, name);
}

Node* NodeFactoryUser::NewRet(Node* control, Node* effect, Node* data) {
  return node_factory_->NewRet(control, effect, data);
}

Node* NodeFactoryUser::NewTuple(Type* output_type) {
  return node_factory_->NewTuple(output_type);
}

Node* NodeFactoryUser::NewTuple(const std::vector<Node*>& inputs) {
  return node_factory_->NewTuple(inputs);
}

// Literal nodes
#define V(Name, mnemonic, data_type)                 \
  Node* NodeFactoryUser::New##Name(data_type data) { \
    return node_factory_->New##Name(data);           \
  }
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

}  // namespace optimizer
}  // namespace elang
