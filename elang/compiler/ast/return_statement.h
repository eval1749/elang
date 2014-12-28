// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_RETURN_STATEMENT_H_
#define ELANG_COMPILER_AST_RETURN_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// ReturnStatement
//
class ReturnStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(ReturnStatement, Statement);
  friend class NodeFactory;

 public:
  Expression* value() const { return value_; }

 private:
  explicit ReturnStatement(Token* keyword, Expression* value);
  ~ReturnStatement() final;

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(ReturnStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_RETURN_STATEMENT_H_
