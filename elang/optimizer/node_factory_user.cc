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

}  // namespace optimizer
}  // namespace elang
