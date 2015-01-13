// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_LOCAL_VARIABLE_H_
#define ELANG_COMPILER_AST_LOCAL_VARIABLE_H_

#include "elang/compiler/ast/nodes.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Variable
//
class Variable final : public NamedNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(Variable, NamedNode);

 public:
  bool is_const() const;
  Expression* type() const { return type_; }
  Expression* value() const { return value_; }

 private:
  // |keyword| one of 'catch', 'const', 'for', 'using', or |nullptr|
  Variable(Token* keyword, Expression* type, Token* name, Expression* value);

  Expression* const type_;
  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_LOCAL_VARIABLE_H_
