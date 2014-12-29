// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_LOCAL_VARIABLE_H_
#define ELANG_COMPILER_AST_LOCAL_VARIABLE_H_

#include "elang/compiler/ast/named_node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// LocalVariable
//
class LocalVariable final : public NamedNode {
  DECLARE_CASTABLE_CLASS(LocalVariable, NamedNode);
  friend class NodeFactory;

 public:
  bool is_const() const;
  Expression* type() const { return type_; }
  Expression* value() const { return value_; }

 private:
  LocalVariable(Token* keyword,
                Expression* type,
                Token* name,
                Expression* value);

  ~LocalVariable() final;

  Expression* const type_;
  Expression* const value_;

  DISALLOW_COPY_AND_ASSIGN(LocalVariable);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_LOCAL_VARIABLE_H_
