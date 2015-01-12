// Copyright 2014 Project Vogue. All rights reserved.
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
class Class final : public ContainerNode, public WithModifiers {
  DECLARE_AST_NODE_CONCRETE_CLASS(Class, ContainerNode);

 public:
  const ZoneVector<Expression*>& base_class_names() const {
    return base_class_names_;
  }

  bool is_class() const;
  bool is_interface() const;
  bool is_struct() const;

  void AddBaseClassName(Expression* class_name);

 private:
  Class(Zone* zone,
        ContainerNode* outer,
        Modifiers modifiers,
        Token* keyword,
        Token* name);

  // Node
  bool is_type() const final;

#if _DEBUG
  // Node
  virtual bool CanBeMemberOf(ContainerNode* container) const;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  ZoneVector<Expression*> base_class_names_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

//////////////////////////////////////////////////////////////////////
//
// Field
//
class Field final : public NamedNode, public WithModifiers {
  DECLARE_AST_NODE_CONCRETE_CLASS(Field, NamedNode);

 public:
  Expression* expression() const { return expression_; }
  Expression* type() const { return type_; }

 private:
  Field(Class* outer,
        Modifiers modifiers,
        Expression* Type,
        Token* name,
        Expression* expression);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  Expression* const expression_;
  Expression* const type_;

  DISALLOW_COPY_AND_ASSIGN(Field);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CLASS_H_
