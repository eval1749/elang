// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_VAR_STATEMENT_H_
#define ELANG_COMPILER_AST_VAR_STATEMENT_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/statement.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// VarStatement
//
class VarStatement final : public Statement {
  DECLARE_AST_NODE_CLASS(VarStatement, Statement);

 public:
  const ZoneVector<LocalVariable*>& variables() const { return variables_; }

 private:
  // |type_token| comes from variable type node.
  VarStatement(Zone* zone,
               Token* type_token,
               const std::vector<LocalVariable*>& variables);

  // Node
  void Accept(Visitor* visitor) override;

  const ZoneVector<LocalVariable*> variables_;

  DISALLOW_COPY_AND_ASSIGN(VarStatement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_VAR_STATEMENT_H_
