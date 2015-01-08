// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPES_H_
#define ELANG_HIR_TYPES_H_

#include <ostream>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/float_types.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_allocated.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/types_forward.h"
#include "elang/hir/values_forward.h"

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
class ELANG_HIR_EXPORT Type : public Castable,
                              public Visitable<TypeVisitor>,
                              public ZoneAllocated {
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

  // Returns default value of this type.
  virtual Value* GetDefaultValue() const = 0;

 protected:
  Type() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

// Print for formatting and debugging.
ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Type& type);

#define DECLARE_HIR_TYPE_CONCRETE_CLASS(self, super) \
  DECLARE_HIR_TYPE_CLASS(self, super)                \
 private:                                            \
  void Accept(TypeVisitor* visitor) override;

// A base class of primitive types.
class ELANG_HIR_EXPORT PrimitiveType : public Type {
  DECLARE_HIR_TYPE_CLASS(PrimitiveType, Type);

 public:
  virtual int bit_size() const = 0;

 protected:
  PrimitiveType() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(PrimitiveType);
};

#define DECLARE_HIR_PRIMITIVE_TYPE(Name, name, value_type, ...)      \
  class ELANG_HIR_EXPORT Name##Type final : public PrimitiveType {   \
    DECLARE_HIR_TYPE_CONCRETE_CLASS(Name##Type, PrimitiveType);      \
                                                                     \
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
    Value* GetDefaultValue() const override;                         \
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
class ELANG_HIR_EXPORT ReferenceType : public Type {
  DECLARE_HIR_TYPE_CLASS(ReferenceType, Type);

 public:
 protected:
  explicit ReferenceType(Zone* zone);

 private:
  // Type
  Value* GetDefaultValue() const override;

  NullLiteral* const null_literal_;

  DISALLOW_COPY_AND_ASSIGN(ReferenceType);
};

// A concrete class represents function type which has return type and parameter
// types.
class ELANG_HIR_EXPORT FunctionType : public ReferenceType {
  DECLARE_HIR_TYPE_CONCRETE_CLASS(FunctionType, ReferenceType);

 public:
  Type* parameters_type() const { return parameters_type_; }
  Type* return_type() const { return return_type_; }

 private:
  FunctionType(Zone* zone, Type* return_type, Type* parameters_type);

  Type* const parameters_type_;
  Type* const return_type_;

  DISALLOW_COPY_AND_ASSIGN(FunctionType);
};

//////////////////////////////////////////////////////////////////////
//
// StringType
//
class ELANG_HIR_EXPORT StringType final : public ReferenceType {
  DECLARE_HIR_TYPE_CONCRETE_CLASS(StringType, ReferenceType);

 public:
  StringLiteral* NewLiteral(base::StringPiece16 data);

 private:
  explicit StringType(Zone* zone);

  DISALLOW_COPY_AND_ASSIGN(StringType);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPES_H_
