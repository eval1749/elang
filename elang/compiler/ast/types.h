// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_TYPES_H_
#define ELANG_COMPILER_AST_TYPES_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/expressions.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Types
//
class Type : public Expression {
  DECLARE_ABSTRACT_AST_NODE_CLASS(Type, Expression);

 protected:
  explicit Type(Token* op);

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

// Represents ArrayType expression:
//  PrimaryExpresion Rank+
//  Rank ::= '[' ','* ']'
class ArrayType final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(ArrayType, Type);

 public:
  Expression* element_type() const { return element_type_; }
  const ZoneVector<int>& dimensions() const { return dimensions_; }

 private:
  ArrayType(Zone* zone,
            Token* op_token,
            Type* element_type,
            const std::vector<int>& dimensions);

  const ZoneVector<int> dimensions_;
  Type* const element_type_;

  DISALLOW_COPY_AND_ASSIGN(ArrayType);
};

// ConstructedType
class ConstructedType final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(ConstructedType, Type);

 public:
  ConstructedName* reference() const { return reference_; }

 private:
  explicit ConstructedType(ConstructedName* reference);

  ConstructedName* const reference_;

  DISALLOW_COPY_AND_ASSIGN(ConstructedType);
};

// InvalidType
class InvalidType final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(InvalidType, Type);

 public:
  Expression* expression() const { return expression_; }

 private:
  explicit InvalidType(Expression* expression);

  Expression* const expression_;

  DISALLOW_COPY_AND_ASSIGN(InvalidType);
};

// OptionalType
class OptionalType final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(OptionalType, Type);

 public:
  Type* base_type() const { return base_type_; }

 private:
  OptionalType(Token* op, Type* base_type);

  Type* const base_type_;

  DISALLOW_COPY_AND_ASSIGN(OptionalType);
};

// TypeMemberAccess
class TypeMemberAccess final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(TypeMemberAccess, Type);

 public:
  MemberAccess* reference() const { return reference_; }

 private:
  explicit TypeMemberAccess(MemberAccess* reference);

  MemberAccess* const reference_;

  DISALLOW_COPY_AND_ASSIGN(TypeMemberAccess);
};

// TypeNameReference
class TypeNameReference final : public Type {
  DECLARE_CONCRETE_AST_NODE_CLASS(TypeNameReference, Type);

 public:
  Token* name() const;
  NameReference* reference() const { return reference_; }

 private:
  explicit TypeNameReference(NameReference* reference);

  NameReference* const reference_;

  DISALLOW_COPY_AND_ASSIGN(TypeNameReference);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_TYPES_H_
