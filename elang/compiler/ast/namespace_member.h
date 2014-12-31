// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NAMESPACE_MEMBER_H_
#define ELANG_COMPILER_AST_NAMESPACE_MEMBER_H_

#include <vector>

#include "elang/compiler/ast/named_node.h"

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
class NamespaceMember : public NamedNode {
  DECLARE_AST_NODE_CLASS(NamespaceMember, NamedNode);

 public:
  Modifiers modifiers() const { return modifiers_; }
  NamespaceBody* namespace_body() const { return namespace_body_; }
  Namespace* outer() const;
  Namespace* owner() const { return outer(); }

  bool IsDescendantOf(const NamespaceMember* other) const;
  virtual Namespace* ToNamespace();

 protected:
  NamespaceMember(NamespaceBody* namespace_body,
                  Modifiers modifiers,
                  Token* keyword_or_name,
                  Token* simple_name);

 private:
  const Modifiers modifiers_;
  // We use |namespace_body_| for name resolution to look up alias and
  // import.
  NamespaceBody* const namespace_body_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAMESPACE_MEMBER_H_
