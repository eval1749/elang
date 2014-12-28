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

 public:
  ~Namespace() override;

  const std::vector<NamespaceBody*> bodies() const { return bodies_; }

  void AddMember(NamespaceMember* member);
  void AddNamespaceBody(NamespaceBody* outer);
  NamespaceMember* FindMember(hir::SimpleName* simple_name);
  NamespaceMember* FindMember(Token* simple_name);

  // NamespaceMember
  Namespace* ToNamespace() override;

 protected:
  Namespace(NamespaceBody* namespace_body, Modifiers modifiers, Token* keyword,
            Token* simple_name);

 private:
  Namespace(NamespaceBody* namespace_body, Token* keyword, Token* simple_name);

  // Node
  void Accept(Visitor* visitor) override;

  struct ImportDef;

  std::vector<NamespaceBody*> bodies_;
  std::unordered_map<hir::SimpleName*, NamespaceMember*> map_;

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_namespace_h)

