// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TYPE_FACTORY_H_
#define ELANG_OPTIMIZER_TYPE_FACTORY_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "elang/base/zone_owner.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/types_forward.h"

namespace elang {
class AtomicString;
namespace optimizer {

class Factory;
struct FactoryConfig;

//////////////////////////////////////////////////////////////////////
//
// TypeFactory
//
class ELANG_OPTIMIZER_EXPORT TypeFactory final : public ZoneOwner {
 public:
  explicit TypeFactory(const FactoryConfig& config);
  ~TypeFactory();

  Type* control_type() const { return control_type_; }
  Type* effect_type() const { return effect_type_; }
  Type* string_type() const { return string_type_; }
// Predefined types" bool_type(), char_type(), and so on.
#define V(Name, name, ...) Name##Type* name##_type() const;
  FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V

  ArrayType* NewArrayType(Type* element_type,
                          const std::vector<int>& dimensions);
  ControlType* NewControlType(Type* data_type);
  ExternalType* NewExternalType(AtomicString* name);
  FunctionType* NewFunctionType(Type* return_type, Type* parameters_type);
  PointerType* NewPointerType(Type* pointee);
  TupleType* NewTupleType(const std::vector<Type*>& components);

 private:
  class ArrayTypeFactory;
  class FunctionTypeFactory;
  class TupleTypeFactory;

  Type* const effect_type_;
  Type* const string_type_;
#define V(Name, name, ...) Name##Type* const name##_type_;
  FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V

  std::unique_ptr<ArrayTypeFactory> array_type_factory_;
  std::unordered_map<Type*, ControlType*> control_type_map_;
  std::unique_ptr<FunctionTypeFactory> function_type_factory_;
  std::unordered_map<Type*, PointerType*> pointer_type_map_;
  std::unique_ptr<TupleTypeFactory> tuple_type_factory_;

  // Cache of NewControlType(void_type())
  Type* const control_type_;

  DISALLOW_COPY_AND_ASSIGN(TypeFactory);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TYPE_FACTORY_H_
