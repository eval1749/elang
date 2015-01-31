// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/type_factory_user.h"

#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace hir {

TypeFactoryUser::TypeFactoryUser(TypeFactory* factory) : factory_(factory) {
}

TypeFactoryUser::~TypeFactoryUser() {
}

#define V(Name, name, ...) \
  Type* TypeFactoryUser::name##_type() const { return types()->name##_type(); }
FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

Type* TypeFactoryUser::string_type() const {
  return types()->string_type();
}

}  // namespace hir
}  // namespace elang
