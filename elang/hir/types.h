// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPES_H_
#define ELANG_HIR_TYPES_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/float_types.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_vector.h"
#include "elang/base/zone_allocated.h"
#include "elang/hir/thing.h"
#include "elang/hir/types_forward.h"
#include "elang/hir/values_forward.h"

namespace elang {
class AtomicString;
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
//    TupleType

#define DECLARE_HIR_TYPE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);      \
  friend class TypeFactory;                 \
                                            \
 protected:                                 \
  ~self() override = default;

#define DECLARE_HIR_TYPE_ABSTRACT_CLASS(self, super) \
  DECLARE_HIR_TYPE_CLASS(self, super)

#define DECLARE_HIR_TYPE_CONCRETE_CLASS(self, super) \
  DECLARE_HIR_TYPE_CLASS(self, super)                \
 private:                                            \
  void Accept(TypeVisitor* visitor) override;

enum class Signedness {
  Unsigned,
  Signed,
};

//////////////////////////////////////////////////////////////////////
//
// Represent HIR type.
//
class ELANG_HIR_EXPORT Type : public Thing, public Visitable<TypeVisitor> {
  DECLARE_HIR_TYPE_ABSTRACT_CLASS(Type, Thing);

 public:
  // |RegiserClass::Integer| and |Register::General| are equivalent.
  enum RegisterClass {
    Float,
    General,
    Integer,
    Tuple,
    Void,
  };

  bool can_allocate_on_stack() const;

  // Returns default value of this type.
  virtual Value* default_value() const;

  bool is_float() const { return register_class() == RegisterClass::Float; }
  bool is_general() const { return register_class() == RegisterClass::General; }
  bool is_integer() const { return register_class() == RegisterClass::Integer; }
  bool is_numeric() const { return is_integer() || is_float(); }
  bool is_void() const { return register_class() == RegisterClass::Void; }

  // Which type of register holds a value of this type.
  virtual RegisterClass register_class() const;

 protected:
  Type() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

// ArrayType is a concrete class represent array.
// - dimensions_[k] == -1 means dimension at rank k is unbound.
// - dimensions_[k] must not be zero except for k == 0 and rank == 1
class ELANG_HIR_EXPORT ArrayType final : public Type {
  DECLARE_HIR_TYPE_CONCRETE_CLASS(ArrayType, Type);

 public:
  Value* default_value() const final;
  Type* element_type() const { return element_type_; }
  const ZoneVector<int> dimensions() const { return dimensions_; }
  int rank() const { return static_cast<int>(dimensions_.size()); }

 private:
  explicit ArrayType(Zone* zone,
                     Type* element_type,
                     const std::vector<int>& dimensions);

  const ZoneVector<int> dimensions_;
  Type* const element_type_;
  NullLiteral* const null_literal_;

  DISALLOW_COPY_AND_ASSIGN(ArrayType);
};

// A concrete class represents function type which has return type and parameter
// types.
class ELANG_HIR_EXPORT FunctionType final : public Type {
  DECLARE_HIR_TYPE_CONCRETE_CLASS(FunctionType, Type);

 public:
  Type* parameters_type() const { return parameters_type_; }
  Type* return_type() const { return return_type_; }

 private:
  FunctionType(Type* return_type, Type* parameters_type);

  Type* const parameters_type_;
  Type* const return_type_;

  DISALLOW_COPY_AND_ASSIGN(FunctionType);
};

//////////////////////////////////////////////////////////////////////
//
// PointerType
//
class ELANG_HIR_EXPORT PointerType final : public Type {
  DECLARE_HIR_TYPE_CONCRETE_CLASS(PointerType, Type);

 public:
  Type* pointee() const { return pointee_; }
  Value* default_value() const final;
  RegisterClass register_class() const final;

 private:
  PointerType(Zone* zone, Type* pointee);

  NullLiteral* const null_literal_;
  Type* pointee_;

  DISALLOW_COPY_AND_ASSIGN(PointerType);
};

//////////////////////////////////////////////////////////////////////
//
// Primitive Types
//
// A base class of primitive types.
class ELANG_HIR_EXPORT PrimitiveType : public Type {
  DECLARE_HIR_TYPE_ABSTRACT_CLASS(PrimitiveType, Type);

 public:
  // TODO(eval1749) Move |PrimitiveType::bit_size()| to |PrimitiveValueType|.
  virtual int bit_size() const = 0;

 protected:
  PrimitiveType() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(PrimitiveType);
};

// PrimitiveValueType is a base class for Int{8,16,32,64}Type,
// Float{32,64}Type, and so on.
class ELANG_HIR_EXPORT PrimitiveValueType : public PrimitiveType {
  DECLARE_HIR_TYPE_ABSTRACT_CLASS(PrimitiveValueType, PrimitiveType);

 public:
  bool is_signed() const { return signedness() == Signedness::Signed; }
  bool is_unsigned() const { return signedness() == Signedness::Unsigned; }

 protected:
  PrimitiveValueType() = default;

  virtual Signedness signedness() const = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(PrimitiveValueType);
};

#define V(Name, name, value_type, ...)                                  \
  class ELANG_HIR_EXPORT Name##Type final : public PrimitiveValueType { \
    DECLARE_HIR_TYPE_CONCRETE_CLASS(Name##Type, PrimitiveValueType);    \
                                                                        \
   public:                                                              \
    /* Protocol defined by |PrimitiveType| class */                     \
    int bit_size() const final;                                         \
    Value* default_value() const final;                                 \
    RegisterClass register_class() const final;                         \
                                                                        \
   private:                                                             \
    /* Allow |Factory| to access |NewLiteral()|. */                     \
    friend class Factory;                                               \
    friend class TypeFactory;                                           \
    /* Since primitive types exist only one instance per factory. */    \
    /* Only |TypeFactory| can construct them. */                        \
    Name##Type(Zone* zone);                                             \
                                                                        \
    Signedness signedness() const final;                                \
                                                                        \
    /* Primitive types are factory of |Literal| objects. */             \
    Name##Literal* NewLiteral(Zone* zone, value_type data);             \
                                                                        \
    ZoneUnorderedMap<value_type, Name##Literal*> literal_cache_;        \
    Name##Literal* const default_value_;                                \
                                                                        \
    DISALLOW_COPY_AND_ASSIGN(Name##Type);                               \
  };
FOR_EACH_HIR_PRIMITIVE_VALUE_TYPE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// ReferenceType
//
class ELANG_HIR_EXPORT ReferenceType : public Type {
  DECLARE_HIR_TYPE_ABSTRACT_CLASS(ReferenceType, Type);

 public:
  // For |FunctionType|, |name()| return |nullptr|.
  AtomicString* name() const { return name_; }

  // Type
  Value* default_value() const override;
  RegisterClass register_class() const final;

 protected:
  explicit ReferenceType(Zone* zone, AtomicString* name);

 private:
  AtomicString* const name_;
  NullLiteral* const null_literal_;

  DISALLOW_COPY_AND_ASSIGN(ReferenceType);
};

// A concrete class represents class, interface, and struct.
class ELANG_HIR_EXPORT ExternalType final : public ReferenceType {
  DECLARE_HIR_TYPE_CONCRETE_CLASS(ExternalType, ReferenceType);

 private:
  ExternalType(Zone* zone, AtomicString* name);

  DISALLOW_COPY_AND_ASSIGN(ExternalType);
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
  StringType(Zone* zone, AtomicString* name);

  DISALLOW_COPY_AND_ASSIGN(StringType);
};

//////////////////////////////////////////////////////////////////////
//
// TupleType
//
class ELANG_HIR_EXPORT TupleType final : public Type {
  DECLARE_HIR_TYPE_CONCRETE_CLASS(TupleType, Type);

 public:
  Type* get(int index) const { return members_[index]; }
  const ZoneVector<Type*>& members() const { return members_; }
  int size() const { return static_cast<int>(members_.size()); }

 private:
  TupleType(Zone* zone, const std::vector<Type*>& members);

  const ZoneVector<Type*> members_;

  DISALLOW_COPY_AND_ASSIGN(TupleType);
};

// VoidType
class ELANG_HIR_EXPORT VoidType final : public PrimitiveType {
  DECLARE_HIR_TYPE_CONCRETE_CLASS(VoidType, PrimitiveType);

 private:
  explicit VoidType(Zone* zone);

  int bit_size() const final;
  Value* default_value() const final;

  Value* const default_value_;

  DISALLOW_COPY_AND_ASSIGN(VoidType);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPES_H_
