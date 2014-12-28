// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_METHOD_H_
#define ELANG_COMPILER_AST_METHOD_H_

#include <vector>

#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/ast/type_and_name.h"
#include "elang/compiler/modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Method
//
class Method final : public Node {
  DECLARE_CASTABLE_CLASS(Method, Node);
  friend class NodeFactory;

 public:
  MethodGroup* method_group() const { return method_group_; }
  Modifiers modifiers() const { return modifiers_; }
  Token* name() const { return token(); }
  const std::vector<TypeAndName>& parameters() const { return parameters_; }
  Expression* return_type() const { return return_type_; }
  const std::vector<Token*>& type_parameters() {  return type_parameters_; }

 private:
  Method(NamespaceBody* namespace_body, MethodGroup* method_group,
         Modifiers modifies, Expression* return_type, Token* name,
         const std::vector<Token*>& type_parameters,
         const std::vector<TypeAndName>& parameters);
  ~Method() final;

  MethodGroup* const method_group_;
  const Modifiers modifiers_;
  NamespaceBody* const namespace_body_;
  const std::vector<TypeAndName> parameters_;
  Expression* const return_type_;
  const std::vector<Token*> type_parameters_;

  DISALLOW_COPY_AND_ASSIGN(Method);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_METHOD_H_
