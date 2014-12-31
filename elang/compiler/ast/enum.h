// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_ENUM_H_
#define ELANG_COMPILER_AST_ENUM_H_

#include "elang/base/zone_unordered_map.h"

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace hir {
class SimpleName;
}
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
  DECLARE_AST_NODE_CLASS(Enum, NamespaceMember);

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

  // Node
  void Accept(Visitor* visitor) override;

  ZoneUnorderedMap<hir::SimpleName*, EnumMember*> map_;
  ZoneVector<EnumMember*> members_;

  DISALLOW_COPY_AND_ASSIGN(Enum);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_ENUM_H_
