// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_LOCAL_VARIABLE_H_
#define ELANG_COMPILER_AST_LOCAL_VARIABLE_H_

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// LocalVariable
//
class LocalVariable final : public NamedNode {
  DECLARE_AST_NODE_CONCRETE_CLASS(LocalVariable, NamedNode);

 public:
  bool is_const() const;
  Expression* type() const { return type_; }
  Expression* value() const { return value_; }

 private:
  // |keyword| one of 'catch', 'const', 'for', 'using', or |nullptr|
  LocalVariable(Token* keyword,
                Expression* type,
                Token* name,
                Expression* value);

  Expression* const type_;
  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(LocalVariable);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_LOCAL_VARIABLE_H_
