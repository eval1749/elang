// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_namespae_body_h)
#define INCLUDE_elang_compiler_ast_namespae_body_h

#include <unordered_map>

#include "elang/compiler/ast/namespace_member.h"

namespace elang {
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
  private: struct ImportDef;

  // TODO(eval1749) Use |AstVector| instead of |std::vector|
  private: std::vector<Alias*> aliases_;
  private: std::unordered_map<Token::SimpleNameId, Alias*> alias_map_;
  private: std::vector<ImportDef*> import_defs_;
  private: std::vector<NamespaceMember*> members_;
  private: NamespaceBody* const outer_;
  private: Namespace* const owner_;

  public: NamespaceBody(NamespaceBody* outer, Namespace* owner);
  public: ~NamespaceBody();

  public: const std::vector<Alias*>& aliases() const;
  public: const std::vector<NamespaceMember*>& members() const {
    return members_;
  }
  public: NamespaceBody* outer() const { return outer_; }
  public: Namespace* owner() const { return owner_; }

  public: void AddImport(const Token& import_keyword,
                         const QualifiedName& name);
  public: void AddAlias(Alias* alias);
  public: void AddMember(NamespaceMember* member);
  public: Alias* FindAlias(const Token& simple_name);
  public: NamespaceMember* FindMember(const Token& simple_name);

  DISALLOW_COPY_AND_ASSIGN(NamespaceBody);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_namespae_body_h)
