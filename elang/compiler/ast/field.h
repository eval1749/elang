// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_FIELD_H_
#define ELANG_COMPILER_AST_FIELD_H_

#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Field
//
class Field final : public NamespaceMember {
  DECLARE_AST_NODE_CONCRETE_CLASS(Field, NamespaceMember);

 public:
  Expression* expression() const { return expression_; }
  Expression* type() const { return type_; }

 private:
  Field(NamespaceBody* namespace_body,
        Modifiers modifiers,
        Expression* Type,
        Token* name,
        Expression* expression);

  // NamespaceMember
  void AcceptMemberVisitor(MemberVisitor* visitor) final;

  Expression* const expression_;
  Expression* const type_;

  DISALLOW_COPY_AND_ASSIGN(Field);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_FIELD_H_
