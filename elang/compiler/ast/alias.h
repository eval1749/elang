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

  private: const QualifiedName target_name_;
  private: NamespaceMember* target_;

  private: Alias(NamespaceBody* namespace_body, Token* keyword,
                 Token* simple_name,
                 const QualifiedName& reference_name);
  public: ~Alias() final;

  public: NamespaceMember* target() const { return target_; }
  public: const QualifiedName& target_name() const { return target_name_; }

  public: void BindTo(NamespaceMember* target);

  // Node
  private: void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(Alias);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_alias_h)

