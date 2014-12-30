// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NAMESPACE_BODY_H_
#define ELANG_COMPILER_AST_NAMESPACE_BODY_H_

#include <unordered_map>
#include <vector>

#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {

namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
class NamespaceBody final {
 public:
  NamespaceBody(NamespaceBody* outer, Namespace* owner);
  ~NamespaceBody();

  const std::vector<Alias*>& aliases() const;
  const std::vector<Import*>& imports() const;
  const std::vector<NamespaceMember*>& members() const { return members_; }
  NamespaceBody* outer() const { return outer_; }
  Namespace* owner() const { return owner_; }

  void AddAlias(Alias* alias);
  void AddImport(Import* import);
  void AddMember(NamespaceMember* member);
  Alias* FindAlias(Token* simple_name);
  Import* FindImport(Token* simple_name);
  NamespaceMember* FindMember(Token* simple_name);

 private:
  struct ImportDef;

  // TODO(eval1749) Use |AstVector| instead of |std::vector|
  std::vector<Alias*> aliases_;
  std::unordered_map<hir::SimpleName*, Alias*> alias_map_;
  std::vector<Import*> imports_;
  std::unordered_map<hir::SimpleName*, Import*> import_map_;
  std::vector<NamespaceMember*> members_;
  NamespaceBody* const outer_;
  Namespace* const owner_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBody);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAMESPACE_BODY_H_
