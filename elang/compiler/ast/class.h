// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CLASS_H_
#define ELANG_COMPILER_AST_CLASS_H_

#include <vector>

#include "elang/compiler/ast/container_node.h"
#include "elang/compiler/ast/with_modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public NamespaceNode, public WithModifiers {
  DECLARE_CONCRETE_AST_NODE_CLASS(Class, NamespaceNode);

 public:
  bool is_class() const;
  bool is_interface() const;
  bool is_struct() const;

 private:
  Class(Zone* zone,
        NamespaceNode* outer,
        Modifiers modifiers,
        Token* keyword,
        Token* name);

#if _DEBUG
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  DISALLOW_COPY_AND_ASSIGN(Class);
};

//////////////////////////////////////////////////////////////////////
//
// ClassBody
//
class ClassBody final : public BodyNode, public WithModifiers {
  DECLARE_CONCRETE_AST_NODE_CLASS(ClassBody, BodyNode);

 public:
  const ZoneVector<Type*>& base_class_names() const {
    return base_class_names_;
  }

  Class* owner() const;

 private:
  ClassBody(Zone* zone,
            BodyNode* outer,
            Class* owner,
            const std::vector<Type*>& base_class_names);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  const ZoneVector<Type*> base_class_names_;

  DISALLOW_COPY_AND_ASSIGN(ClassBody);
};

//////////////////////////////////////////////////////////////////////
//
// Const
//
class Const final : public NamedNode, public WithModifiers {
  DECLARE_CONCRETE_AST_NODE_CLASS(Const, NamedNode);

 public:
  Expression* expression() const { return expression_; }
  Type* type() const { return type_; }

 private:
  Const(ClassBody* outer,
        Modifiers modifiers,
        Token* keyword,
        Type* Type,
        Token* name,
        Expression* expression);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  Expression* const expression_;
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Const);
};

//////////////////////////////////////////////////////////////////////
//
// Field
//
class Field final : public NamedNode, public WithModifiers {
  DECLARE_CONCRETE_AST_NODE_CLASS(Field, NamedNode);

 public:
  Expression* expression() const { return expression_; }
  Type* type() const { return type_; }

 private:
  Field(ClassBody* outer,
        Modifiers modifiers,
        Token* keyword,
        Type* Type,
        Token* name,
        Expression* expression);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  Expression* const expression_;
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Field);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CLASS_H_
