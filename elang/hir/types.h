// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPES_H_
#define ELANG_HIR_TYPES_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/types.h"
#include "elang/base/zone_allocated.h"
#include "elang/hir/instructions_forward.h"
#include "elang/hir/types_forward.h"

namespace elang {
namespace hir {

// See "types_forward.h" for list of all types.

// Type class hierarchy
//  Type
//    ArrayType
//    FunctionType
//    PointerType
//    PrimitiveType
//      Float32Type Float64Type
//      Int16Type Int32Type Int64Type Int8Type
//      UInt16Type UInt32Type UInt64Type UInt8Type
//      VoidType
//    ReferenceType
//      StringType
//    StructureType

#define DECLARE_HIR_TYPE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);      \
  friend class TypeFactory;                 \
                                            \
 protected:                                 \
  ~self() override = default;

//////////////////////////////////////////////////////////////////////
//
// Represent HIR type.
//
class Type : public Castable, public ZoneAllocated {
  DECLARE_HIR_TYPE_CLASS(Type, Castable);

 public:
  enum RegisterClass {
    Float,
    General,
    Void,
  };

  bool is_float() const { return register_class() == RegisterClass::Float; }
  bool is_general() const { return register_class() == RegisterClass::General; }
  bool is_void() const { return register_class() == RegisterClass::Void; }
  // Which type of register holds a value of this type.
  virtual RegisterClass register_class() const;

  // Returns |NullLiteral| if this type is nullable, otherwise returns null.
  virtual NullLiteral* GetNullLiteral() const;

 protected:
  Type() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

// A base class of primitive types.
class PrimitiveType : public Type {
  DECLARE_HIR_TYPE_CLASS(PrimitiveType, Type);

 public:
  virtual int bit_size() const = 0;

 protected:
  PrimitiveType() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(PrimitiveType);
};

#define DECLARE_HIR_PRIMITIVE_TYPE(Name, name, value_type, ...)      \
  class Name##Type final : public PrimitiveType {                    \
   public:                                                           \
    Name##Literal* zero() const { return zero_; }                    \
                                                                     \
    /* Primitive types are factory of |Literal| objects. */          \
    Name##Literal* NewLiteral(value_type data);                      \
                                                                     \
   private:                                                          \
    friend class TypeFactory;                                        \
    /* Since primitive types exist only one instance per factory. */ \
    /* Only |TypeFactory| can construct them. */                     \
    Name##Type(Zone* zone);                                          \
                                                                     \
    /* Protocol defined by |PrimitiveType| class */                  \
    int bit_size() const override;                                   \
    RegisterClass register_class() const override;                   \
                                                                     \
    /* |zone_| should be initialized before other members. */        \
    Zone* const zone_;                                               \
    ZoneUnorderedMap<value_type, Name##Literal*> literal_cache_;     \
    Name##Literal* const zero_;                                      \
                                                                     \
    DISALLOW_COPY_AND_ASSIGN(Name##Type);                            \
  };
FOR_EACH_HIR_PRIMITIVE_TYPE(DECLARE_HIR_PRIMITIVE_TYPE)
#undef DECLARE_HIR_PRIMITIVE_TYPE

//////////////////////////////////////////////////////////////////////
//
// ReferenceType
//
class ReferenceType : public Type {
  DECLARE_HIR_TYPE_CLASS(ReferenceType, Type);

 public:
  NullLiteral* GetNullLiteral() const override;

 protected:
  explicit ReferenceType(Zone* zone);

 private:
  NullLiteral* const null_literal_;

  DISALLOW_COPY_AND_ASSIGN(ReferenceType);
};

//////////////////////////////////////////////////////////////////////
//
// StringType
//
class StringType final : public ReferenceType {
  DECLARE_HIR_TYPE_CLASS(StringType, ReferenceType);

 public:
  StringLiteral* NewLiteral(base::StringPiece16 data);

 private:
  explicit StringType(Zone* zone);

  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(StringType);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPES_H_
