// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/factory_user.h"

#include "elang/optimizer/factory.h"

namespace elang {
namespace optimizer {

FactoryUser::FactoryUser(Factory* factory)
    : NodeFactoryUser(factory->node_factory()),
      TypeFactoryUser(factory->type_factory()),
      factory_(factory) {
}

FactoryUser::~FactoryUser() {
}

const std::vector<ErrorData*>& FactoryUser::errors() const {
  return factory_->errors();
}

Zone* FactoryUser::zone() const {
  return factory_->zone();
}

AtomicString* FactoryUser::NewAtomicString(base::StringPiece16 data) {
  return factory_->NewAtomicString(data);
}

Function* FactoryUser::NewFunction(FunctionType* function_type) {
  return factory_->NewFunction(function_type);
}

}  // namespace optimizer
}  // namespace elang
