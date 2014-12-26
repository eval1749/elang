// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_namespace_member_h)
#define INCLUDE_elang_compiler_ast_namespace_member_h

#include <vector>

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

class Namespace;
class NamespaceBody;

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
class NamespaceMember : public Node {
  DECLARE_CASTABLE_CLASS(NamespaceMember, Node);

  private: NamespaceBody* const alias_declaration_space_;
  private: Namespace* const outer_;
  private: const Token simple_name_;

  public: NamespaceMember(Namespace* owner, const Token& keyword_or_name,
                          const Token& simple_name);
  public: ~NamespaceMember() override;

  public: NamespaceBody* alias_declaration_space() const {
    return alias_declaration_space_;
  }
  public: Namespace* outer() const { return outer_; }
  public: const Token& simple_name() const { return simple_name_; }

  public: bool IsDescendantOf(const NamespaceMember* other) const;
  public: virtual Namespace* ToNamespace();

  DISALLOW_COPY_AND_ASSIGN(NamespaceMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_namespace_member_h)

