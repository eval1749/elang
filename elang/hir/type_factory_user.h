// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPE_FACTORY_USER_H_
#define ELANG_HIR_TYPE_FACTORY_USER_H_

#include "base/macros.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/types_forward.h"

namespace elang {
namespace hir {

class Type;
class TypeFactory;

//////////////////////////////////////////////////////////////////////
//
// TypeFactoryUser
//
class ELANG_HIR_EXPORT TypeFactoryUser {
 public:
  ~TypeFactoryUser();

  TypeFactory* types() const { return factory_; }

#define V(Name, name, ...) Type* name##_type() const;
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
  Type* string_type() const;

 protected:
  explicit TypeFactoryUser(TypeFactory* factory);

 private:
  TypeFactory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(TypeFactoryUser);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPE_FACTORY_USER_H_
