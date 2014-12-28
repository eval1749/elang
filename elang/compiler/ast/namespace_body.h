// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_namespae_body_h)
#define INCLUDE_elang_compiler_ast_namespae_body_h

#include <unordered_map>
#include <vector>

#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {
class QualifiedName;

namespace ast {
class Alias;
class NamespaceMember;
class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
class NamespaceBody final {
 public:
  NamespaceBody(NamespaceBody* outer, Namespace* owner);
  ~NamespaceBody();

  const std::vector<Alias*>& aliases() const;
  const std::vector<NamespaceMember*>& members() const {
    return members_;
  }
  NamespaceBody* outer() const { return outer_; }
  Namespace* owner() const { return owner_; }

  void AddImport(Token* import_keyword, const QualifiedName& name);
  void AddAlias(Alias* alias);
  void AddMember(NamespaceMember* member);
  Alias* FindAlias(Token* simple_name);
  NamespaceMember* FindMember(Token* simple_name);

 private:
  struct ImportDef;

  // TODO(eval1749) Use |AstVector| instead of |std::vector|
  std::vector<Alias*> aliases_;
  std::unordered_map<hir::SimpleName*, Alias*> alias_map_;
  std::vector<ImportDef*> import_defs_;
  std::vector<NamespaceMember*> members_;
  NamespaceBody* const outer_;
  Namespace* const owner_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBody);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_namespae_body_h)
