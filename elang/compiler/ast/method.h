// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_METHOD_H_
#define ELANG_COMPILER_AST_METHOD_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/container_node.h"
#include "elang/compiler/ast/with_modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Method contains type parameters in named map.
//
class Method final : public NamespaceNode, public WithModifiers {
  DECLARE_CONCRETE_AST_NODE_CLASS(Method, Node);

 public:
  // Returns method body. Its is null when parsing is failed or |extern|
  // method.
  Statement* body() const { return body_; }

  MethodGroup* method_group() const { return method_group_; }
  Class* owner() const;
  const ZoneVector<Variable*>& parameters() const { return parameters_; }
  Type* return_type() const { return return_type_; }

  // Type parameters for generic method.
  const ZoneVector<Token*>& type_parameters() { return type_parameters_; }

 private:
  Method(Zone* zone,
         ClassBody* owner,
         MethodGroup* method_group,
         Modifiers modifies,
         Type* return_type,
         Token* name,
         const std::vector<Token*>& type_parameters,
         const std::vector<Variable*>& parameters,
         Statement* body);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
#endif

  // |body_| can be |BlockStatement| or |ExpressionStatement|, by shortcut
  // syntax |int Foo(int x) => x + 1;|. It can be |nullptr| on parsing error,
  // or abstract/external method.
  Statement* const body_;
  MethodGroup* const method_group_;
  const ZoneVector<Variable*> parameters_;
  Type* const return_type_;
  const ZoneVector<Token*> type_parameters_;

  DISALLOW_COPY_AND_ASSIGN(Method);
};

//////////////////////////////////////////////////////////////////////
//
// MethodGroup
//
class MethodGroup final : public NamedNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(MethodGroup, NamedNode);

 public:
  const ZoneVector<Method*>& methods() const { return methods_; }
  Class* owner() const;

  void AddMethod(Method* method);

 private:
  MethodGroup(Zone* zone, Class* owner, Token* name);

#if _DEBUG
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  ZoneVector<Method*> methods_;

  DISALLOW_COPY_AND_ASSIGN(MethodGroup);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_METHOD_H_
