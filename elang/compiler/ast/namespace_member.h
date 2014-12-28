// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NAMESPACE_MEMBER_H_
#define ELANG_COMPILER_AST_NAMESPACE_MEMBER_H_

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

 public:
  Modifiers modifiers() const { return modifiers_; }
  Token* name() const { return simple_name_; }
  NamespaceBody* namespace_body() const { return namespace_body_; }
  Namespace* outer() const;
  Namespace* owner() const { return outer(); }
  // TODO(eval1749) Get rid of |NamespaceMember::simple_name()|.
  Token* simple_name() const { return name(); }

  bool IsDescendantOf(const NamespaceMember* other) const;
  virtual Namespace* ToNamespace();

 protected:
  NamespaceMember(NamespaceBody* namespace_body,
                  Modifiers modifiers,
                  Token* keyword_or_name,
                  Token* simple_name);
  ~NamespaceMember() override;

 private:
  const Modifiers modifiers_;
  // We use |namespace_body_| for name resolution to look up alias and
  // import.
  NamespaceBody* const namespace_body_;
  Token* const simple_name_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAMESPACE_MEMBER_H_
