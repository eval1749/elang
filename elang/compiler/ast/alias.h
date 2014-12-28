// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_alias_h)
#define INCLUDE_elang_compiler_ast_alias_h

#include "elang/compiler/ast/namespace_member.h"

#include "elang/compiler/qualified_name.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Alias
//
class Alias final : public NamespaceMember {
  DECLARE_CASTABLE_CLASS(Alias, NamespaceMember);

  friend class NodeFactory;

 public:
  ~Alias() final;

  NamespaceMember* target() const { return target_; }
  const QualifiedName& target_name() const { return target_name_; }

  void BindTo(NamespaceMember* target);

 private:
  Alias(NamespaceBody* namespace_body, Token* keyword,
                 Token* simple_name,
                 const QualifiedName& reference_name);

  // Node
  void Accept(Visitor* visitor) override;

  const QualifiedName target_name_;
  NamespaceMember* target_;

  DISALLOW_COPY_AND_ASSIGN(Alias);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_alias_h)

