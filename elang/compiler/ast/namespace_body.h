// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NAMESPACE_BODY_H_
#define ELANG_COMPILER_AST_NAMESPACE_BODY_H_

#include <unordered_map>
#include <vector>

#include "elang/base/zone_allocated.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
class NamespaceBody final : public ZoneAllocated {
 public:
  const ZoneVector<Alias*>& aliases() const;
  const ZoneVector<Import*>& imports() const;
  const ZoneVector<NamespaceMember*>& members() const { return members_; }
  NamespaceBody* outer() const { return outer_; }
  Namespace* owner() const { return owner_; }

  void AddAlias(Alias* alias);
  void AddImport(Import* import);
  void AddMember(NamespaceMember* member);
  Alias* FindAlias(Token* simple_name);
  Import* FindImport(Token* simple_name);
  NamespaceMember* FindMember(Token* simple_name);

 private:
  friend class NodeFactory;
  NamespaceBody(Zone* zone, NamespaceBody* outer, Namespace* owner);
  ~NamespaceBody() = delete;

  // TODO(eval1749) Use |AstVector| instead of |ZoneVector|
  ZoneVector<Alias*> aliases_;
  ZoneUnorderedMap<AtomicString*, Alias*> alias_map_;
  ZoneVector<Import*> imports_;
  ZoneUnorderedMap<AtomicString*, Import*> import_map_;
  ZoneVector<NamespaceMember*> members_;
  NamespaceBody* const outer_;
  Namespace* const owner_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBody);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAMESPACE_BODY_H_
