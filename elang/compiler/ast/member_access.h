// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_MEMBER_ACCESS_H_
#define ELANG_COMPILER_AST_MEMBER_ACCESS_H_

#include <vector>

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// MemberAccess
//
// Represents type components connected by '.', e.g. |G<S, T>.F<X>.A|.
//
class MemberAccess final : public Expression {
  DECLARE_CASTABLE_CLASS(MemberAccess, Expression);

  friend class NodeFactory;

 public:
  ~MemberAccess() final;

  const std::vector<Expression*>& components() const { return components_; }

 private:
  MemberAccess(Token* name, const std::vector<Expression*>& components);

  // Node
  void Accept(Visitor* visitor) override;

  const std::vector<Expression*> components_;

  DISALLOW_COPY_AND_ASSIGN(MemberAccess);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_MEMBER_ACCESS_H_
