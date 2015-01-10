// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_ENUM_H_
#define ELANG_COMPILER_AST_ENUM_H_

#include "elang/base/zone_unordered_map.h"

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/namespace_member.h"

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
class Enum final : public NamespaceMember {
  DECLARE_AST_NODE_CONCRETE_CLASS(Enum, NamespaceMember);

 public:
  const ZoneVector<EnumMember*> members() const { return members_; }

  void AddMember(EnumMember* member);
  EnumMember* FindMember(Token* simple_name);

 private:
  Enum(Zone* zone,
       NamespaceBody* namespace_body,
       Modifiers modifies,
       Token* keyword,
       Token* name);

  // NamespaceMember
  void AcceptMemberVisitor(MemberVisitor* visitor) final;

  ZoneUnorderedMap<AtomicString*, EnumMember*> map_;
  ZoneVector<EnumMember*> members_;

  DISALLOW_COPY_AND_ASSIGN(Enum);
};

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
class EnumMember final : public NamedNode {
  DECLARE_AST_NODE_CLASS(EnumMember, NamedNode);

 public:
  Expression* expression() const { return expression_; }

 private:
  EnumMember(Enum* owner, Token* name, Expression* expression);

  Expression* expression_;

  DISALLOW_COPY_AND_ASSIGN(EnumMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_ENUM_H_
