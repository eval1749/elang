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

  // We use |namespace_body_| for name resolution to look up alias and
  // import.
  private: NamespaceBody* const namespace_body_;
  private: Token* const simple_name_;

  protected: NamespaceMember(NamespaceBody* namespace_body,
                             Token* keyword_or_name,
                             Token* simple_name);
  protected: ~NamespaceMember() override;

  public: NamespaceBody* namespace_body() const { return namespace_body_; }
  public: Namespace* outer() const;
  public: Namespace* owner() const { return outer(); }
  public: Token* simple_name() const { return simple_name_; }

  public: bool IsDescendantOf(const NamespaceMember* other) const;
  public: virtual Namespace* ToNamespace();

  DISALLOW_COPY_AND_ASSIGN(NamespaceMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_namespace_member_h)

