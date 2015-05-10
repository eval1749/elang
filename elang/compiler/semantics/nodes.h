// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SEMANTICS_NODES_H_
#define ELANG_COMPILER_SEMANTICS_NODES_H_

#include <vector>

#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_unordered_set.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/semantics/nodes_forward.h"

namespace elang {
namespace compiler {
namespace sm {

#define DECLARE_SEMANTIC_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);      \
                                            \
 protected:                                 \
  ~self() = default;

#define DECLARE_ABSTRACT_SEMANTIC_CLASS(self, super) \
  DECLARE_SEMANTIC_CLASS(self, super);

#define DECLARE_CONCRETE_SEMANTIC_CLASS(self, super) \
  DECLARE_SEMANTIC_CLASS(self, super);               \
                                                     \
 private:                                            \
  friend class Editor;                               \
  friend class Factory;                              \
  /* Visitor pattern */                              \
  void Accept(Visitor* visitor) final;

#define FOR_EACH_STORAGE_CLASS(V) \
  V(Heap)                         \
  V(Local)                        \
  V(NonLocal)                     \
  V(ReadOnly)                     \
  V(Void)

enum class StorageClass {
#define V(Name) Name,
  FOR_EACH_STORAGE_CLASS(V)
#undef V
};

//////////////////////////////////////////////////////////////////////
//
// Semantic
//
class Semantic : public Castable<Semantic>,
                 public Visitable<Visitor>,
                 public ZoneAllocated {
  DECLARE_ABSTRACT_SEMANTIC_CLASS(Semantic, Castable);

 public:
  virtual Token* name() const;
  virtual Semantic* outer() const;
  Token* token() const { return token_; }

 protected:
  explicit Semantic(Token* token);

 private:
  Token* const token_;

  DISALLOW_COPY_AND_ASSIGN(Semantic);
};

//////////////////////////////////////////////////////////////////////
//
// Value
//
class Value : public Semantic {
  DECLARE_ABSTRACT_SEMANTIC_CLASS(Value, Semantic);

 public:
  Type* type() const { return type_; }

 protected:
  Value(Type* type, Token* token);

 private:
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Value);
};

//////////////////////////////////////////////////////////////////////
//
// Type
//
class Type : public Semantic {
  DECLARE_ABSTRACT_SEMANTIC_CLASS(Type, Semantic);

 public:
  virtual bool IsSubtypeOf(const Type* other) const = 0;

 protected:
  explicit Type(Token* token);

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

template <typename Base>
class NamedMember : public Base {
 public:
  Token* name() const final { return name_; }
  Semantic* outer() const final { return outer_; }

 protected:
  NamedMember(Semantic* outer, Token* name)
      : Base(name), name_(name), outer_(outer) {}

 private:
  Token* const name_;
  Semantic* const outer_;

  DISALLOW_COPY_AND_ASSIGN(NamedMember);
};

//////////////////////////////////////////////////////////////////////
//
// ArrayType
//
class ArrayType final : public Type {
  DECLARE_CONCRETE_SEMANTIC_CLASS(ArrayType, Type);

 public:
  // Dimension of each rank. dimensions.front() == -1 means unbound array.
  const ZoneVector<int>& dimensions() const { return dimensions_; }
  Type* element_type() const { return element_type_; }
  int rank() const { return static_cast<int>(dimensions_.size()); }

 private:
  ArrayType(Zone* zone, Type* element_type, const std::vector<int>& dimensions);

  // Type
  bool IsSubtypeOf(const Type* other) const final;

  const ZoneVector<int> dimensions_;
  Type* const element_type_;

  DISALLOW_COPY_AND_ASSIGN(ArrayType);
};

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public NamedMember<Type> {
  DECLARE_CONCRETE_SEMANTIC_CLASS(Class, Type);

 public:
  // Associated AST class object.
  ast::Class* ast_class() const { return ast_class_; }

  // base classes of this class
  const ZoneUnorderedSet<Class*>& base_classes() const;

  // Direct base classes of this class
  const ZoneVector<Class*>& direct_base_classes() const;

  bool has_base() const { return has_base_; }

  // Returns true if |this| is 'class'.
  bool is_class() const;

  Semantic* FindMember(Token* name) const;

 private:
  Class(Zone* zone, Semantic* outer, Token* name, ast::Class* ast_type);

  // Type
  bool IsSubtypeOf(const Type* other) const final;

  ast::Class* const ast_class_;
  ZoneUnorderedSet<Class*> base_classes_;
  ZoneVector<Class*> direct_base_classes_;
  bool has_base_;
  ZoneUnorderedMap<AtomicString*, Semantic*> members_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

//////////////////////////////////////////////////////////////////////
//
// Enum
//
class Enum final : public NamedMember<Type> {
  DECLARE_CONCRETE_SEMANTIC_CLASS(Enum, Type);

 public:
  sm::Type* enum_base() const;
  const ZoneVector<EnumMember*>& members() const { return members_; }
  bool has_base() const { return enum_base_ != nullptr; }

 private:
  Enum(Zone* zone, Semantic* outer, Token* name);

  // Type
  bool IsSubtypeOf(const Type* other) const final;

  sm::Type* enum_base_;
  ZoneVector<EnumMember*> members_;

  DISALLOW_COPY_AND_ASSIGN(Enum);
};

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
class EnumMember final : public NamedMember<Semantic> {
  DECLARE_CONCRETE_SEMANTIC_CLASS(EnumMember, Semantic);

 public:
  bool has_value() const { return value_ != nullptr; }
  Enum* owner() const { return outer()->as<Enum>(); }
  Value* value() const;

 private:
  EnumMember(Enum* owner, Token* name);

  Value* value_;

  DISALLOW_COPY_AND_ASSIGN(EnumMember);
};

//////////////////////////////////////////////////////////////////////
//
// InvalidValue
//
class InvalidValue final : public Value {
  DECLARE_CONCRETE_SEMANTIC_CLASS(InvalidValue, Value);

 private:
  InvalidValue(Type* type, Token* token);

  DISALLOW_COPY_AND_ASSIGN(InvalidValue);
};

//////////////////////////////////////////////////////////////////////
//
// Literal
//
class Literal final : public Value {
  DECLARE_CONCRETE_SEMANTIC_CLASS(Literal, Value);

 public:
  Token* data() const { return data_; }

 private:
  Literal(Type* type, Token* token);

  Token* const data_;

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

//////////////////////////////////////////////////////////////////////
//
// Method
//
class Method final : public NamedMember<Semantic> {
  DECLARE_CONCRETE_SEMANTIC_CLASS(Method, Semantic);

 public:
  MethodGroup* method_group() const { return method_group_; }
  const ZoneVector<Parameter*>& parameters() const;
  Type* return_type() const;
  Signature* signature() const { return signature_; }

 private:
  Method(MethodGroup* method_group, Signature* signature);

  MethodGroup* const method_group_;
  Signature* const signature_;

  DISALLOW_COPY_AND_ASSIGN(Method);
};

//////////////////////////////////////////////////////////////////////
//
// MethodGroup
//
class MethodGroup final : public NamedMember<Semantic> {
  DECLARE_CONCRETE_SEMANTIC_CLASS(MethodGroup, Semantic);

 public:
  const ZoneVector<Method*>& methods() const { return methods_; }
  Class* owner() const { return outer()->as<Class>(); }

 private:
  MethodGroup(Zone* zone, Class* owner, Token* name);

  ZoneVector<Method*> methods_;

  DISALLOW_COPY_AND_ASSIGN(MethodGroup);
};

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
class Namespace final : public NamedMember<Semantic> {
  DECLARE_CONCRETE_SEMANTIC_CLASS(Namespace, Semantic);

 public:
  Semantic* FindMember(Token* name) const;

 private:
  Namespace(Zone* zone, Namespace* outer, Token* name);

  ZoneUnorderedMap<AtomicString*, Semantic*> members_;

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

//////////////////////////////////////////////////////////////////////
//
// Parameter
//
class Parameter final : public Semantic {
  DECLARE_CONCRETE_SEMANTIC_CLASS(Parameter, Semantic);

 public:
  bool operator==(const Parameter& other) const;
  bool operator!=(const Parameter& other) const;

  Value* default_value() const { return default_value_; }
  bool is_rest() const;
  ParameterKind kind() const;
  int position() const;
  Type* type() const { return type_; }

  bool IsIdentical(const Parameter& other) const;

  // Node
  Token* name() const final;

 private:
  Parameter(ast::Parameter* ast_parameter, Type* type, Value* default_value);

  ast::Parameter* const ast_parameter_;
  Value* const default_value_;
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Parameter);
};

//////////////////////////////////////////////////////////////////////
//
// Signature
//
class Signature final : public Type {
  DECLARE_CONCRETE_SEMANTIC_CLASS(Signature, Type);

 public:
  struct Arity {
    int maximum;
    int minimum;
    bool is_rest;
  };

  bool operator==(const Signature& other) const;
  bool operator!=(const Signature& other) const;

  int maximum_arity() const { return arity_.minimum; }
  int minimum_arity() const { return arity_.maximum; }
  const ZoneVector<Parameter*>& parameters() const { return parameters_; }
  Type* return_type() { return return_type_; }

  bool IsIdenticalParameters(const Signature* other) const;

 private:
  Signature(Zone* zone,
            Type* return_type,
            const std::vector<Parameter*>& parameters);

  // Type
  bool IsSubtypeOf(const Type* other) const final;

  const Arity arity_;
  const ZoneVector<Parameter*> parameters_;
  Type* const return_type_;

  DISALLOW_COPY_AND_ASSIGN(Signature);
};

//////////////////////////////////////////////////////////////////////
//
// UndefinedType
//
class UndefinedType final : public Type {
  DECLARE_CONCRETE_SEMANTIC_CLASS(UndefinedType, Type);

 private:
  explicit UndefinedType(Token* token);

  bool IsSubtypeOf(const Type* other) const final;

  DISALLOW_COPY_AND_ASSIGN(UndefinedType);
};

//////////////////////////////////////////////////////////////////////
//
// Variable
//
class Variable final : public Semantic {
  DECLARE_CONCRETE_SEMANTIC_CLASS(Variable, Semantic);

 public:
  ast::NamedNode* ast_node() const { return ast_node_; }
  StorageClass storage() const { return storage_; }
  Type* type() const { return type_; }

 private:
  Variable(Type* type, StorageClass storage, ast::NamedNode* ast_node);

  ast::NamedNode* const ast_node_;
  StorageClass const storage_;
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace sm
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SEMANTICS_NODES_H_
