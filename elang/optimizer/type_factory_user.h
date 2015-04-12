// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TYPE_FACTORY_USER_H_
#define ELANG_OPTIMIZER_TYPE_FACTORY_USER_H_

#include <vector>

#include "base/macros.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/types_forward.h"

namespace elang {
class AtomicString;

namespace optimizer {

class Type;
class TypeFactory;

//////////////////////////////////////////////////////////////////////
//
// TypeFactoryUser
//
class ELANG_OPTIMIZER_EXPORT TypeFactoryUser {
 public:
  ~TypeFactoryUser();

  Type* control_type() const;
  Type* effect_type() const;
  Type* string_type() const;
  TypeFactory* type_factory() const { return type_factory_; }
#define V(Name, name, ...) Type* name##_type() const;
  FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V

  ArrayType* NewArrayType(Type* element_type,
                          const std::vector<int>& dimensions);
  ControlType* NewControlType(Type* data_type);
  ExternalType* NewExternalType(AtomicString* name);
  FunctionType* NewFunctionType(Type* return_type, Type* parameters_type);
  PointerType* NewPointerType(Type* pointee);
  TupleType* NewTupleType(const std::vector<Type*>& members);

 protected:
  explicit TypeFactoryUser(TypeFactory* type_factory);

 private:
  TypeFactory* const type_factory_;

  DISALLOW_COPY_AND_ASSIGN(TypeFactoryUser);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TYPE_FACTORY_USER_H_
