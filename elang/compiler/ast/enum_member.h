// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_enum_member_h)
#define INCLUDE_elang_compiler_ast_enum_member_h

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
class EnumMember final : public Node {
  DECLARE_CASTABLE_CLASS(EnumMember, Node);

  friend class NodeFactory;

 public:
  ~EnumMember() final;

  Expression* expression() const { return expression_; }
  Token* name() const { return token(); }

 private:
  EnumMember(Enum* owner, Token* name, Expression* expression);

  Expression* expression_;

  DISALLOW_COPY_AND_ASSIGN(EnumMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_enum_member_h)
