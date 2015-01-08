// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_STATEMENT_H_
#define ELANG_COMPILER_AST_STATEMENT_H_

#include <vector>

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Statement
//
class Statement : public Node {
  DECLARE_AST_NODE_CLASS(Statement, Node);

 public:
  Token* keyword() const { return token(); }

 protected:
  explicit Statement(Token* keyword);

 private:
  DISALLOW_COPY_AND_ASSIGN(Statement);
};

// Represents comma separated expressions used in 'for' statement.
class ExpressionList : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ExpressionList, Statement);

 public:
  const std::vector<Expression*>& expressions() const { return expressions_; }

 protected:
  ExpressionList(Token* keyword, const std::vector<Expression*>& expressions);

 private:
  const std::vector<Expression*> expressions_;

  DISALLOW_COPY_AND_ASSIGN(ExpressionList);
};

// Represents 'for' + ':' statement.
class ForEachStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ForEachStatement, Statement);

 public:
  Expression* enumerable() const { return enumerable_; }
  Statement* statement() const { return statement_; }
  LocalVariable* variable() const { return variable_; }

 private:
  ForEachStatement(Token* keyword,
                   LocalVariable* variable,
                   Expression* enumerable,
                   Statement* statement);

  Expression* const enumerable_;
  Statement* const statement_;
  LocalVariable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(ForEachStatement);
};

// Represents 'for' statement.
class ForStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(ForStatement, Statement);

 public:
  Expression* condition() const { return condition_; }
  Statement* initializer() const { return initializer_; }
  Statement* statement() const { return statement_; }
  Statement* step() const { return step_; }

 private:
  ForStatement(Token* keyword,
               Statement* initializer,
               Expression* condition,
               Statement* step,
               Statement* statement);

  Expression* const condition_;
  Statement* const initializer_;
  Statement* const statement_;
  Statement* const step_;

  DISALLOW_COPY_AND_ASSIGN(ForStatement);
};

// Represents invalid statement. This statement is used for continuing parsing
// after syntax error.
class InvalidStatement final : public Statement {
  DECLARE_AST_NODE_CONCRETE_CLASS(InvalidStatement, Statement);

 private:
  explicit InvalidStatement(Token* token);

  DISALLOW_COPY_AND_ASSIGN(InvalidStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_STATEMENT_H_
