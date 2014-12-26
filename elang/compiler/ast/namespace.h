// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_namespace_h)
#define INCLUDE_elang_compiler_ast_namespace_h

#include <vector>

#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {
class QualifiedName;
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
  private: struct ImportDef;

  private: std::vector<NamespaceBody*> bodies_;
  private: std::unordered_map<hir::SimpleName*, NamespaceMember*> map_;

  protected: Namespace(NamespaceBody* namespace_body,
                       const Token& keyword, const Token& simple_name);
  public: ~Namespace() override;

  public: const std::vector<NamespaceBody*> bodies() const { return bodies_; }

  public: void AddMember(NamespaceMember* member);
  public: void AddNamespaceBody(NamespaceBody* outer);
  public: NamespaceMember* FindMember(hir::SimpleName* simple_name);
  public: NamespaceMember* FindMember(const Token& simple_name);

  // NamespaceMember
  public: Namespace* ToNamespace() override;

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_namespace_h)

