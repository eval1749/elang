// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_namespace_member_h)
#define INCLUDE_elang_compiler_ast_namespace_member_h

#include <vector>

#include "elang/compiler/ast/node.h"

#include "elang/compiler/modifiers.h"

namespace elang {
namespace compiler {
class Modifiers;
namespace ast {

class Namespace;
class NamespaceBody;

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
class NamespaceMember : public Node {
  DECLARE_CASTABLE_CLASS(NamespaceMember, Node);

  private: const Modifiers modifiers_;
  // We use |namespace_body_| for name resolution to look up alias and
  // import.
  private: NamespaceBody* const namespace_body_;
  private: Token* const simple_name_;

  protected: NamespaceMember(NamespaceBody* namespace_body,
                             Modifiers modifiers, Token* keyword_or_name,
                             Token* simple_name);
  protected: ~NamespaceMember() override;

  public: Modifiers modifiers() const { return modifiers_; }
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

