// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_THROW_STATEMENT_H_
#define ELANG_COMPILER_AST_THROW_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// ThrowStatement
//
class ThrowStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(ThrowStatement, Statement);
  friend class NodeFactory;

 public:
  Expression* value() const { return value_; }

 private:
  explicit ThrowStatement(Token* keyword, Expression* value);
  ~ThrowStatement() final;

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(ThrowStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_THROW_STATEMENT_H_
