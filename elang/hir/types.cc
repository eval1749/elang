// Copyright 2014 Project Vogue. All rights reserved.
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

namespace {
base::StringPiece16 NewString(Zone* zone, base::StringPiece16 string_piece) {
  auto const size = string_piece.size() * sizeof(base::char16);
  auto const string = static_cast<base::char16*>(zone->Allocate(size));
  ::memcpy(string, string_piece.data(), size);
  return base::StringPiece16(string, string_piece.size());
}
}  // namespace

#define V(Name, ...)                              \
  void Name##Type::Accept(TypeVisitor* visitor) { \
    visitor->Visit##Name##Type(this);             \
  }
FOR_EACH_HIR_TYPE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Type
//
Type::RegisterClass Type::register_class() const {
  return RegisterClass::General;
}

NullLiteral* Type::GetNullLiteral() const {
  return nullptr;
}

FunctionType::FunctionType(Type* return_type, Type* parameters_type)
    : parameters_type_(parameters_type), return_type_(return_type) {
}

//////////////////////////////////////////////////////////////////////
//
// PrimitiveTypes
//
// TODO(eval1749) NYI literal object cache.
#define V(Name, name, data_type, bit_size_value, kind_value)        \
  PrimitiveType::RegisterClass Name##Type::register_class() const { \
    return kind_value;                                              \
  }                                                                 \
  int Name##Type::bit_size() const { return bit_size_value; }       \
                                                                    \
  /* Constructor of primitive type */                               \
  Name##Type::Name##Type(Zone* zone)                                \
      : zone_(zone), literal_cache_(zone), zero_(NewLiteral(0)) {}  \
                                                                    \
  /* Literal object factory function of primitive type */           \
  Name##Literal* Name##Type::NewLiteral(data_type data) {           \
    auto const it = literal_cache_.find(data);                      \
    if (it != literal_cache_.end())                                 \
      return it->second;                                            \
    auto const new_literal = new (zone_) Name##Literal(this, data); \
    literal_cache_[data] = new_literal;                             \
    return new_literal;                                             \
  }
FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// ReferenceType
//
ReferenceType::ReferenceType(Zone* zone)
    : null_literal_(new (zone) NullLiteral(this)) {
}

NullLiteral* ReferenceType::GetNullLiteral() const {
  DCHECK(null_literal_);
  return null_literal_;
}

StringType::StringType(Zone* zone) : ReferenceType(zone), zone_(zone) {
}

StringLiteral* StringType::NewLiteral(base::StringPiece16 data) {
  return new (zone_) StringLiteral(this, NewString(zone_, data));
}

}  // namespace hir
}  // namespace elang
