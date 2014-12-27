// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_field_h)
#define INCLUDE_elang_compiler_ast_field_h

#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Field
//
class Field final : public NamespaceMember {
  DECLARE_CASTABLE_CLASS(Field, NamespaceMember);

  friend class NodeFactory;

  private: Expression* const expression_;
  private: Expression* const type_;

  private: Field(NamespaceBody* namespace_body, Modifiers modifiers,
                 Expression* Type, Token* name, Expression* expression);
  public: ~Field() final;

  public: Expression* expression() const { return expression_; }
  public: Token* name() const { return token(); }
  public: Expression* type() const { return type_; }

  // Node
  private: void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(Field);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_field_h)
