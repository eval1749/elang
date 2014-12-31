// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CALL_H_
#define ELANG_COMPILER_AST_CALL_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Call
//
class Call final : public Expression {
  DECLARE_AST_NODE_CLASS(Call, Expression);

 public:
  const ZoneVector<Expression*>& arguments() const { return arguments_; }
  Expression* callee() const { return callee_; }

 private:
  Call(Zone* zone,
       Expression* callee,
       const std::vector<Expression*>& arguments);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const callee_;
  const ZoneVector<Expression*> arguments_;

  DISALLOW_COPY_AND_ASSIGN(Call);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CALL_H_
