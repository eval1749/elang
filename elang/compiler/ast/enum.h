// Copyright 2014 Project Vogue. All rights reserved.
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
class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Enum
//
class Enum final : public ContainerNode, WithModifiers {
  DECLARE_AST_NODE_CONCRETE_CLASS(Enum, ContainerNode);

 private:
  Enum(Zone* zone,
       ContainerNode* namespace_body,
       Modifiers modifies,
       Token* keyword,
       Token* name);

  // Node
  bool is_type() const final;

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  DISALLOW_COPY_AND_ASSIGN(Enum);
};

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
class EnumMember final : public NamedNode {
  DECLARE_AST_NODE_CONCRETE_CLASS(EnumMember, NamedNode);

 public:
  Expression* expression() const { return expression_; }

 private:
  EnumMember(Enum* owner, Token* name, Expression* expression);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  Expression* const expression_;

  DISALLOW_COPY_AND_ASSIGN(EnumMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_ENUM_H_
