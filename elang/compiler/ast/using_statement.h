// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_USING_STATEMENT_H_
#define ELANG_COMPILER_AST_USING_STATEMENT_H_

#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// UsingStatement
//
class UsingStatement final : public Statement {
  DECLARE_CASTABLE_CLASS(UsingStatement, Statement);
  friend class NodeFactory;

 public:
  Expression* resource() const { return resource_; }
  Statement* statement() const { return statement_; }
  LocalVariable* variable() const { return variable_; }

 private:
  UsingStatement(Token* keyword,
                 LocalVariable* variable,
                 Expression* resource,
                 Statement* statement);
  ~UsingStatement() final;

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const resource_;
  Statement* const statement_;
  LocalVariable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(UsingStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_USING_STATEMENT_H_
