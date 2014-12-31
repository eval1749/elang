// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_RETURN_STATEMENT_H_
#define ELANG_COMPILER_AST_RETURN_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// ReturnStatement
//
class ReturnStatement final : public Statement {
  DECLARE_AST_NODE_CLASS(ReturnStatement, Statement);

 public:
  Expression* value() const { return value_; }

 private:
  ReturnStatement(Token* keyword, Expression* value);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(ReturnStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_RETURN_STATEMENT_H_
