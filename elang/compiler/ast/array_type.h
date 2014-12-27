// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_array_type_h)
#define INCLUDE_elang_compiler_ast_array_type_h

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

  private: Expression* const element_type_;
  private: const std::vector<int> ranks_;

  private: ArrayType(Token* op_token, Expression* expression,
                     const std::vector<int>& ranks);
  public: ~ArrayType() final;

  public: Expression* element_type() const { return element_type_; }
  public: const std::vector<int>& ranks() const { return ranks_; }

  // Node
  private: void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(ArrayType);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_array_type_h)
