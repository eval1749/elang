// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_ENUM_H_
#define ELANG_COMPILER_AST_ENUM_H_

#include "elang/compiler/ast/container_node.h"
#include "elang/compiler/ast/with_modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

class Class;
class EnumMember;
class Factory;

//////////////////////////////////////////////////////////////////////
//
// Enum
//
class Enum final : public NamespaceNode, WithModifiers {
  DECLARE_CONCRETE_AST_NODE_CLASS(Enum, NamespaceNode);

 public:
  Type* enum_base() const { return enum_base_; }

 private:
  Enum(Zone* zone,
       BodyNode* container,
       Modifiers modifies,
       Token* keyword,
       Token* name,
       Type* enum_base);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  Type* const enum_base_;

  DISALLOW_COPY_AND_ASSIGN(Enum);
};

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
class EnumMember final : public NamedNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(EnumMember, NamedNode);

 public:
  Expression* expression() const { return expression_; }
  int position() const { return position_; }

 private:
  EnumMember(Enum* owner, Token* name, int position, Expression* expression);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  Expression* const expression_;
  int const position_;

  DISALLOW_COPY_AND_ASSIGN(EnumMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_ENUM_H_
