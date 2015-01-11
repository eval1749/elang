// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CLASS_H_
#define ELANG_COMPILER_AST_CLASS_H_

#include <vector>

#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/with_modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public MemberContainer, public WithModifiers {
  DECLARE_AST_NODE_CONCRETE_CLASS(Class, MemberContainer);

 public:
  const ZoneVector<Expression*>& base_class_names() const {
    return base_class_names_;
  }

  bool is_class() const;
  bool is_interface() const;
  bool is_struct() const;

  void AddBaseClassName(Expression* class_name);

 private:
  Class(Zone* zone,
        NamespaceBody* namespace_body,
        Modifiers modifiers,
        Token* keyword,
        Token* name);

  // Node
  bool is_type() const final;

  ZoneVector<Expression*> base_class_names_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CLASS_H_
