// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_ALIAS_H_
#define ELANG_COMPILER_AST_ALIAS_H_

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

  Expression* reference() const { return reference_; }

 private:
  Alias(NamespaceBody* namespace_body,
        Token* keyword,
        Token* alias_name,
        Expression* target_name);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const reference_;

  DISALLOW_COPY_AND_ASSIGN(Alias);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_ALIAS_H_
