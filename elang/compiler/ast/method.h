// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_METHOD_H_
#define ELANG_COMPILER_AST_METHOD_H_

#include <vector>

#include "elang/compiler/ast/named_node.h"
#include "elang/compiler/modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Method
//
class Method final : public NamedNode {
  DECLARE_CASTABLE_CLASS(Method, NamedNode);
  friend class NodeFactory;

 public:
  MethodGroup* method_group() const { return method_group_; }
  Modifiers modifiers() const { return modifiers_; }
  const std::vector<LocalVariable*>& parameters() const { return parameters_; }
  Expression* return_type() const { return return_type_; }

  // Returns method body. Its is null when parsing is failed or |extern|
  // method.
  Statement* statement() const { return statement_; }

  // Type parameters for generic method.
  const std::vector<Token*>& type_parameters() { return type_parameters_; }

  // Set method body to |statement|. |statement| can be |BlockStatement| or
  // |Expression|, by shortcut syntax |int Foo(int x) => x + 1;|.
  // |statement| can't be null.
  void SetStatement(Statement* statement);

 private:
  Method(NamespaceBody* namespace_body,
         MethodGroup* method_group,
         Modifiers modifies,
         Expression* return_type,
         Token* name,
         const std::vector<Token*>& type_parameters,
         const std::vector<LocalVariable*>& parameters);
  ~Method() final;

  MethodGroup* const method_group_;
  const Modifiers modifiers_;
  NamespaceBody* const namespace_body_;
  const std::vector<LocalVariable*> parameters_;
  Expression* const return_type_;
  Statement* statement_;
  const std::vector<Token*> type_parameters_;

  DISALLOW_COPY_AND_ASSIGN(Method);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_METHOD_H_
