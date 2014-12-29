// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_VAR_STATEMENT_H_
#define ELANG_COMPILER_AST_VAR_STATEMENT_H_

#include <vector>

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

  const std::vector<LocalVariable*>& variables() const { return variables_; }

 private:
  VarStatement(Token* name, const std::vector<LocalVariable*>& variables);

  // Node
  void Accept(Visitor* visitor) override;

  const std::vector<LocalVariable*> variables_;

  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_VAR_STATEMENT_H_
