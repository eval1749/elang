// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_YIELD_STATEMENT_H_
#define ELANG_COMPILER_AST_YIELD_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// YieldStatement
//
class YieldStatement final : public Statement {
  DECLARE_AST_NODE_CLASS(YieldStatement, Statement);

 public:
  Expression* value() const { return value_; }

 private:
  YieldStatement(Token* keyword, Expression* value);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(YieldStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_YIELD_STATEMENT_H_
