// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPE_FACTORY_H_
#define ELANG_HIR_TYPE_FACTORY_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "elang/base/zone_owner.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/types_forward.h"

namespace elang {
class AtomicString;
namespace hir {

class Factory;
struct FactoryConfig;

//////////////////////////////////////////////////////////////////////
//
// TypeFactory
//
class ELANG_HIR_EXPORT TypeFactory final : public ZoneOwner {
 public:
  explicit TypeFactory(const FactoryConfig& config);
  ~TypeFactory();

// Predefined types" bool_type(), char_type(), and so on.
#define V(Name, name, ...) Name##Type* name##_type() const;
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
  StringType* string_type() const { return string_type_; }

  ExternalType* NewExternalType(AtomicString* name);
  FunctionType* NewFunctionType(Type* return_type, Type* parameters_type);
  PointerType* NewPointerType(Type* pointee);
  TupleType* NewTupleType(const std::vector<Type*>& members);

 private:
  class FunctionTypeFactory;

#define V(Name, name, ...) Name##Type* const name##_type_;
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

  std::unordered_map<Type*, PointerType*> pointer_type_map_;
  std::unique_ptr<FunctionTypeFactory> function_type_factory_;
  StringType* const string_type_;

  DISALLOW_COPY_AND_ASSIGN(TypeFactory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPE_FACTORY_H_
