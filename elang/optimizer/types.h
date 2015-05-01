// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TYPES_H_
#define ELANG_OPTIMIZER_TYPES_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/float_types.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_vector.h"
#include "elang/base/zone_allocated.h"
#include "elang/optimizer/thing.h"
#include "elang/optimizer/types_forward.h"

#undef RegisterClass

namespace elang {
class AtomicString;
namespace optimizer {

// See "types_forward.h" for list of all types.

// Type class hierarchy
//  Type
//    ArrayType
//    ControlType
//    ControlTupleType
//    EffectType
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

#define DECLARE_OPTIMIZER_TYPE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);            \
  friend class TypeFactory;                       \
                                                  \
 protected:                                       \
  ~self() override = default;

#define DECLARE_OPTIMIZER_TYPE_ABSTRACT_CLASS(self, super) \
  DECLARE_OPTIMIZER_TYPE_CLASS(self, super)

#define DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(self, super) \
  DECLARE_OPTIMIZER_TYPE_CLASS(self, super)                \
 private:                                                  \
  void Accept(TypeVisitor* visitor) final;

enum class Signedness {
  Unsigned,
  Signed,
};

//////////////////////////////////////////////////////////////////////
//
// Represent OPTIMIZER type.
//
class ELANG_OPTIMIZER_EXPORT Type : public Thing,
                                    public Visitable<TypeVisitor> {
  DECLARE_OPTIMIZER_TYPE_ABSTRACT_CLASS(Type, Thing);

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

  bool is_float() const { return register_class() == RegisterClass::Float; }
  bool is_general() const { return register_class() == RegisterClass::General; }
  bool is_integer() const { return register_class() == RegisterClass::Integer; }
  bool is_numeric() const { return is_integer() || is_float(); }
  bool is_signed() const { return signedness() == Signedness::Signed; }
  bool is_unsigned() const { return signedness() == Signedness::Unsigned; }
  bool is_void() const { return register_class() == RegisterClass::Void; }

  // Which type of register holds a value of this type.
  virtual RegisterClass register_class() const;
  virtual Signedness signedness() const;

 protected:
  Type() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

// ArrayType is a concrete class represent array.
// - dimensions_[k] == -1 means dimension at rank k is unbound.
// - dimensions_[k] must not be zero except for k == 0 and rank == 1
class ELANG_OPTIMIZER_EXPORT ArrayType final : public Type {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(ArrayType, Type);

 public:
  Type* element_type() const { return element_type_; }
  const ZoneVector<int> dimensions() const { return dimensions_; }
  size_t rank() const { return dimensions_.size(); }

 private:
  explicit ArrayType(Zone* zone,
                     Type* element_type,
                     const std::vector<int>& dimensions);

  const ZoneVector<int> dimensions_;
  Type* const element_type_;

  DISALLOW_COPY_AND_ASSIGN(ArrayType);
};

//////////////////////////////////////////////////////////////////////
//
// ControlType
//
class ELANG_OPTIMIZER_EXPORT ControlType final : public Type {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(ControlType, Type);

 public:
  Type* data_type() const { return data_type_; }

 private:
  explicit ControlType(Type* data_type);

  Type* const data_type_;

  DISALLOW_COPY_AND_ASSIGN(ControlType);
};

//////////////////////////////////////////////////////////////////////
//
// EffectType
//
class ELANG_OPTIMIZER_EXPORT EffectType final : public Type {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(EffectType, Type);

 private:
  EffectType();

  DISALLOW_COPY_AND_ASSIGN(EffectType);
};

//////////////////////////////////////////////////////////////////////
//
// A concrete class represents function type which has return type and parameter
// types.
class ELANG_OPTIMIZER_EXPORT FunctionType final : public Type {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(FunctionType, Type);

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
class ELANG_OPTIMIZER_EXPORT PointerType final : public Type {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(PointerType, Type);

 public:
  Type* pointee() const { return pointee_; }
  RegisterClass register_class() const final;

 private:
  explicit PointerType(Type* pointee);

  Type* pointee_;

  DISALLOW_COPY_AND_ASSIGN(PointerType);
};

//////////////////////////////////////////////////////////////////////
//
// Primitive Types
//
// A base class of primitive types.
class ELANG_OPTIMIZER_EXPORT PrimitiveType : public Type {
  DECLARE_OPTIMIZER_TYPE_ABSTRACT_CLASS(PrimitiveType, Type);

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
class ELANG_OPTIMIZER_EXPORT PrimitiveValueType : public PrimitiveType {
  DECLARE_OPTIMIZER_TYPE_ABSTRACT_CLASS(PrimitiveValueType, PrimitiveType);

 protected:
  PrimitiveValueType() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(PrimitiveValueType);
};

#define V(Name, ...)                                                          \
  class ELANG_OPTIMIZER_EXPORT Name##Type final : public PrimitiveValueType { \
    DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(Name##Type, PrimitiveValueType);    \
                                                                              \
   public:                                                                    \
    /* Protocol defined by |PrimitiveType| class */                           \
    int bit_size() const final;                                               \
    RegisterClass register_class() const final;                               \
                                                                              \
   private:                                                                   \
    friend class TypeFactory;                                                 \
                                                                              \
    Name##Type();                                                             \
                                                                              \
    Signedness signedness() const final;                                      \
                                                                              \
    DISALLOW_COPY_AND_ASSIGN(Name##Type);                                     \
  };
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// ReferenceType
//
class ELANG_OPTIMIZER_EXPORT ReferenceType : public Type {
  DECLARE_OPTIMIZER_TYPE_ABSTRACT_CLASS(ReferenceType, Type);

 public:
  // For |FunctionType|, |name()| return |nullptr|.
  AtomicString* name() const { return name_; }

  // Type
  RegisterClass register_class() const final;

 protected:
  explicit ReferenceType(AtomicString* name);

 private:
  AtomicString* const name_;

  DISALLOW_COPY_AND_ASSIGN(ReferenceType);
};

// A concrete class represents class, interface, and struct.
class ELANG_OPTIMIZER_EXPORT ExternalType final : public ReferenceType {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(ExternalType, ReferenceType);

 private:
  explicit ExternalType(AtomicString* name);

  DISALLOW_COPY_AND_ASSIGN(ExternalType);
};

//////////////////////////////////////////////////////////////////////
//
// StringType
//
class ELANG_OPTIMIZER_EXPORT StringType final : public ReferenceType {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(StringType, ReferenceType);

 private:
  explicit StringType(AtomicString* name);

  DISALLOW_COPY_AND_ASSIGN(StringType);
};

//////////////////////////////////////////////////////////////////////
//
// TupleType
//
class ELANG_OPTIMIZER_EXPORT TupleType final : public Type {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(TupleType, Type);

 public:
  Type* get(size_t index) const { return components_[index]; }
  const ZoneVector<Type*>& components() const { return components_; }
  size_t size() const { return components_.size(); }

 private:
  TupleType(Zone* zone, const std::vector<Type*>& components);

  const ZoneVector<Type*> components_;

  DISALLOW_COPY_AND_ASSIGN(TupleType);
};

// VoidType
class ELANG_OPTIMIZER_EXPORT VoidType final : public PrimitiveType {
  DECLARE_OPTIMIZER_TYPE_CONCRETE_CLASS(VoidType, PrimitiveType);

 private:
  VoidType();

  int bit_size() const final;

  DISALLOW_COPY_AND_ASSIGN(VoidType);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TYPES_H_
