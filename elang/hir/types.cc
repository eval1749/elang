// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/types.h"

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/type_visitor.h"

namespace elang {
namespace hir {

#define V(Name) \
  void Name::Accept(TypeVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_HIR_CONCRETE_TYPE(V)
#undef V

// ArrayType
ArrayType::ArrayType(Zone* zone,
                     Type* element_type,
                     const std::vector<int>& dimensions)
    : dimensions_(zone, dimensions),
      element_type_(element_type),
      null_literal_(new (zone) NullLiteral(this)) {
#if _DEBUG
  for (auto const dimension : dimensions) {
    DCHECK_GE(dimension, 0);
  }
#endif
}

Value* ArrayType::default_value() const {
  DCHECK(null_literal_);
  return null_literal_;
}

// ExternalType
ExternalType::ExternalType(Zone* zone, AtomicString* name)
    : ReferenceType(zone, name) {
}

// FunctionType
FunctionType::FunctionType(Zone* zone, Type* return_type, Type* parameters_type)
    : ReferenceType(zone, nullptr),
      parameters_type_(parameters_type),
      return_type_(return_type) {
}

// PointerType
PointerType::PointerType(Zone* zone, Type* pointee)
    : null_literal_(new (zone) NullLiteral(this)), pointee_(pointee) {
}

Value* PointerType::default_value() const {
  DCHECK(null_literal_);
  return null_literal_;
}

// PrimitiveTypes
// TODO(eval1749) NYI literal object cache.
#define V(Name, name, data_type, bit_size_value, kind_value)          \
  PrimitiveType::RegisterClass Name##Type::register_class() const {   \
    return kind_value;                                                \
  }                                                                   \
  int Name##Type::bit_size() const { return bit_size_value; }         \
                                                                      \
  /* Constructor of primitive type */                                 \
  Name##Type::Name##Type(Zone* zone)                                  \
      : literal_cache_(zone), default_value_(NewLiteral(zone, 0)) {}  \
                                                                      \
  /* Literal object factory function of primitive type */             \
  Name##Literal* Name##Type::NewLiteral(Zone* zone, data_type data) { \
    auto const it = literal_cache_.find(data);                        \
    if (it != literal_cache_.end())                                   \
      return it->second;                                              \
    auto const new_literal = new (zone) Name##Literal(this, data);    \
    literal_cache_[data] = new_literal;                               \
    return new_literal;                                               \
  }                                                                   \
  /* Type */                                                          \
  Value* Name##Type::default_value() const { return default_value_; }
FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

// ReferenceType
ReferenceType::ReferenceType(Zone* zone, AtomicString* name)
    : name_(name), null_literal_(new (zone) NullLiteral(this)) {
}

Value* ReferenceType::default_value() const {
  DCHECK(null_literal_);
  return null_literal_;
}

// StringType
StringType::StringType(Zone* zone, AtomicString* name)
    : ReferenceType(zone, name) {
}

// TupleType
TupleType::TupleType(Zone* zone, const std::vector<Type*>& members)
    : default_value_(new (zone) TupleLiteral(this)), members_(zone, members) {
  DCHECK_GE(members.size(), 2u);
}

Value* TupleType::default_value() const {
  return default_value_;
}

Type::RegisterClass TupleType::register_class() const {
  return RegisterClass::Tuple;
}

// Type
Type::RegisterClass Type::register_class() const {
  return RegisterClass::General;
}

}  // namespace hir
}  // namespace elang
