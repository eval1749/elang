// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_ENUM_MEMBER_H_
#define ELANG_COMPILER_AST_ENUM_MEMBER_H_

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
class EnumMember final : public NamedNode {
  DECLARE_AST_NODE_CLASS(EnumMember, NamedNode);

 public:
  Expression* expression() const { return expression_; }

 private:
  EnumMember(Enum* owner, Token* name, Expression* expression);

  Expression* expression_;

  DISALLOW_COPY_AND_ASSIGN(EnumMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_ENUM_MEMBER_H_
