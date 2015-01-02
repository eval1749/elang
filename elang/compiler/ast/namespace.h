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
// Namespace
//
class Namespace : public NamespaceMember {
  DECLARE_AST_NODE_CLASS(Namespace, NamespaceMember);

 public:
  const ZoneVector<NamespaceBody*> bodies() const { return bodies_; }

  void AddMember(NamespaceMember* member);
  void AddNamespaceBody(NamespaceBody* outer);
  NamespaceMember* FindMember(AtomicString* simple_name);
  NamespaceMember* FindMember(Token* simple_name);

  // NamespaceMember
  Namespace* ToNamespace() override;

 protected:
  Namespace(Zone* zone,
            NamespaceBody* namespace_body,
            Modifiers modifiers,
            Token* keyword,
            Token* simple_name);

 private:
  Namespace(Zone* zone,
            NamespaceBody* namespace_body,
            Token* keyword,
            Token* name);

  // Node
  void Accept(Visitor* visitor) override;

  struct ImportDef;

  ZoneVector<NamespaceBody*> bodies_;
  ZoneUnorderedMap<AtomicString*, NamespaceMember*> map_;

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAMESPACE_H_
