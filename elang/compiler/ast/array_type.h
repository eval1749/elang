// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_ARRAY_TYPE_H_
#define ELANG_COMPILER_AST_ARRAY_TYPE_H_

#include <vector>

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// ArrayType
//
class ArrayType final : public Expression {
  DECLARE_CASTABLE_CLASS(ArrayType, Expression);

  friend class NodeFactory;

 public:
  ~ArrayType() final;

  Expression* element_type() const { return element_type_; }
  const std::vector<int>& ranks() const { return ranks_; }

 private:
  ArrayType(Token* op_token,
            Expression* expression,
            const std::vector<int>& ranks);

  // Node
  void Accept(Visitor* visitor) override;

  Expression* const element_type_;
  const std::vector<int> ranks_;

  DISALLOW_COPY_AND_ASSIGN(ArrayType);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_ARRAY_TYPE_H_
