// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_member_access_h)
#define INCLUDE_elang_compiler_ast_member_access_h

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
// Represents type members connected by '.', e.g. |G<S, T>.F<X>.A|.
//
class MemberAccess final : public Expression {
  DECLARE_CASTABLE_CLASS(MemberAccess, Expression);

  friend class NodeFactory;

 public:
  ~MemberAccess() final;

  const std::vector<Expression*>& members() const { return members_; }

 private:
  explicit MemberAccess(const std::vector<Expression*>& members);

  // Node
  void Accept(Visitor* visitor) override;

  const std::vector<Expression*> members_;

  DISALLOW_COPY_AND_ASSIGN(MemberAccess);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_member_access_h)