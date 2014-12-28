// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_constructed_type_h)
#define INCLUDE_elang_compiler_ast_constructed_type_h

#include <vector>

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// ConstructedType
//
class ConstructedType final : public Expression {
  DECLARE_CASTABLE_CLASS(ConstructedType, Expression);

  friend class NodeFactory;

 public:
  ~ConstructedType() final;

  const std::vector<Expression*>& arguments() const { return arguments_; }
  Expression* blueprint_type() const { return blueprint_type_; }

 private:
  ConstructedType(Token* op_token, Expression* expression,
                  const std::vector<Expression*>& arguments);
  // Node
  void Accept(Visitor* visitor) override;

  const std::vector<Expression*> arguments_;
  Expression* const blueprint_type_;

  DISALLOW_COPY_AND_ASSIGN(ConstructedType);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_constructed_type_h)
