// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NAMESPACE_H_
#define ELANG_COMPILER_AST_NAMESPACE_H_

#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// MemberContainer
//
class MemberContainer : public NamespaceMember {
  DECLARE_AST_NODE_CLASS(MemberContainer, NamespaceMember);

 public:
  const ZoneVector<NamespaceBody*> bodies() const { return bodies_; }
  const ZoneUnorderedMap<AtomicString*, NamespaceMember*> name_map() const {
    return map_;
  }

  void AddMember(NamespaceMember* member);
  void AddNamespaceBody(NamespaceBody* outer);
  NamespaceMember* FindMember(AtomicString* simple_name);
  NamespaceMember* FindMember(Token* simple_name);

 protected:
  MemberContainer(Zone* zone,
                  NamespaceBody* namespace_body,
                  Modifiers modifiers,
                  Token* keyword,
                  Token* simple_name);

 private:
  ZoneVector<NamespaceBody*> bodies_;
  ZoneUnorderedMap<AtomicString*, NamespaceMember*> map_;

  DISALLOW_COPY_AND_ASSIGN(MemberContainer);
};

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
class Namespace final : public MemberContainer {
  DECLARE_AST_NODE_CONCRETE_CLASS(Namespace, MemberContainer);

 private:
  Namespace(Zone* zone,
            NamespaceBody* namespace_body,
            Token* keyword,
            Token* name);

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAMESPACE_H_
