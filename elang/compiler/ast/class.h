// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CLASS_H_
#define ELANG_COMPILER_AST_CLASS_H_

#include <vector>

#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/qualified_name.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public Namespace {
  DECLARE_AST_NODE_CLASS(Class, Namespace);

 public:
  const ZoneVector<Expression*>& base_class_names() const {
    return base_class_names_;
  }

  void AddBaseClassName(Expression* class_name);

  // NamespaceMember
  Namespace* ToNamespace() final;

 private:
  Class(Zone* zone,
        NamespaceBody* namespace_body,
        Modifiers modifiers,
        Token* keyword,
        Token* name);

  // Node
  void Accept(Visitor* visitor) override;

  ZoneVector<Expression*> base_class_names_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CLASS_H_
