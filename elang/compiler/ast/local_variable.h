// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_LOCAL_VARIABLE_H_
#define ELANG_COMPILER_AST_LOCAL_VARIABLE_H_

#include "elang/compiler/ast/statement.h"

#include "elang/compiler/modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// LocalVariable
//
class LocalVariable final : public Node {
  DECLARE_CASTABLE_CLASS(LocalVariable, Node);
  friend class NodeFactory;

 public:
  ~LocalVariable() final;

  bool is_const() const;
  Token* name() const { return token(); }
  Expression* type() const { return type_; }
  Expression* value() const { return value_; }

 private:
  explicit LocalVariable(Token* keyword,
                         Expression* type,
                         Token* name,
                         Expression* value);

  Token* const keyword_;
  Expression* const type_;
  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(LocalVariable);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_LOCAL_VARIABLE_H_
