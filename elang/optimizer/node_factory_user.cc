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

#define V(Name, mnemonic, data_type)                 \
  Node* NodeFactoryUser::New##Name(data_type data) { \
    return node_factory_->New##Name(data);           \
  }
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

}  // namespace optimizer
}  // namespace elang
