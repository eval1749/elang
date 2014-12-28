// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_VAR_STATEMENT_H_
#define ELANG_COMPILER_AST_VAR_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// VarStatement
//
class VarStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(VarStatement, Statement);
  friend class NodeFactory;

 public:
  ~VarStatement() final;

  Token* name() const { return token(); }
  Expression* type() const { return type_; }
  Expression* value() const { return value_; }

 private:
  explicit VarStatement(Expression* type, Token* name, Expression* value);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const type_;
  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_VAR_STATEMENT_H_
