// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_namespace_h)
#define INCLUDE_elang_compiler_ast_namespace_h

#include <vector>

#include "elang/compiler/ast/namespace_member.h"

#include "elang/compiler/qualified_name.h"

namespace elang {
namespace compiler {
namespace ast {

class NamespaceBody;
class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
class Namespace : public NamespaceMember {
  DECLARE_CASTABLE_CLASS(Namespace, NamespaceMember);

  friend class NodeFactory;

  public: struct AliasDef {
    Token alias_name;
    QualifiedName real_name;
    AliasDef(const Token& alias_name, QualifiedName&& real_name);
  };

  public: struct ImportDef {
    Token keyword;
    QualifiedName name;
    ImportDef(const Token& keyword, QualifiedName&& real_name);
  };

  private: std::vector<AliasDef> alias_defs_;
  private: std::vector<ImportDef> import_defs_;
  private: std::unordered_map<Token::SimpleNameId, NamespaceMember*> map_;
  private: std::vector<NamespaceMember*> members_;
  private: QualifiedName name_;

  protected: Namespace(Namespace* outer, const Token& keyword,
                       const Token& simple_name);
  private: Namespace(Namespace* outer, const Token& keyword,
                     QualifiedName&& name);
  public: ~Namespace() override;

  public: std::vector<NamespaceMember*> members() const {
    return members_;
  }
  public: const QualifiedName& name() const { return name_; }

  public: void AddAlias(const Token& alias_name, QualifiedName&& real_name);
  public: void AddImport(const Token& import_keyword, QualifiedName&& name);
  public: void AddMember(NamespaceMember* member);
  public: NamespaceMember* FindMember(const Token& simple_name);

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_namespace_h)

