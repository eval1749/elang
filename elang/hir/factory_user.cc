// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/factory_user.h"

#include "elang/hir/factory.h"

namespace elang {
namespace hir {

FactoryUser::FactoryUser(Factory* factory)
    : TypeFactoryUser(factory->types()), factory_(factory) {
}

FactoryUser::~FactoryUser() {
}

Value* FactoryUser::false_value() const {
  return factory()->false_value();
}

Value* FactoryUser::true_value() const {
  return factory()->true_value();
}

Value* FactoryUser::void_value() const {
  return factory()->void_value();
}

Zone* FactoryUser::zone() const {
  return factory()->zone();
}

}  // namespace hir
}  // namespace elang
