// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/type_factory.h"

#include "elang/base/zone.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {

TypeFactory::TypeFactory()
    : zone_(new Zone()),
#define V(Name, name, ...) name##_type_(new (zone()) Name##Type(zone())),
      FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
          string_type_(new (zone()) StringType(zone())) {
}

#define V(Name, name, ...) \
  Name##Type* TypeFactory::Get##Name##Type() const { return name##_type_; }
FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

}  // namespace hir
}  // namespace elang
