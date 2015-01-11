// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_METHOD_H_
#define ELANG_COMPILER_AST_METHOD_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Method
//
class Method final : public NamespaceMember {
  DECLARE_AST_NODE_CONCRETE_CLASS(Method, NamespaceMember);

 public:
  // Returns method body. Its is null when parsing is failed or |extern|
  // method.
  Statement* body() const { return body_; }

  MethodGroup* method_group() const { return method_group_; }
  const ZoneVector<LocalVariable*>& parameters() const { return parameters_; }
  Expression* return_type() const { return return_type_; }

  // Type parameters for generic method.
  const ZoneVector<Token*>& type_parameters() { return type_parameters_; }

  // Set method body to |statement|. |statement| can be |BlockStatement| or
  // |Expression|, by shortcut syntax |int Foo(int x) => x + 1;|.
  // |statement| can't be null.
  void SetBody(Statement* statement);

 private:
  Method(Zone* zone,
         NamespaceBody* namespace_body,
         MethodGroup* method_group,
         Modifiers modifies,
         Expression* return_type,
         Token* name,
         const std::vector<Token*>& type_parameters,
         const std::vector<LocalVariable*>& parameters);

  Statement* body_;
  MethodGroup* const method_group_;
  const ZoneVector<LocalVariable*> parameters_;
  Expression* const return_type_;
  const ZoneVector<Token*> type_parameters_;

  DISALLOW_COPY_AND_ASSIGN(Method);
};

//////////////////////////////////////////////////////////////////////
//
// MethodGroup
//
class MethodGroup final : public NamespaceMember {
  DECLARE_AST_NODE_CLASS(MethodGroup, NamespaceMember);

 public:
  const ZoneVector<Method*>& methods() const { return methods_; }

  void AddMethod(Method* method);

 private:
  MethodGroup(Zone* zone, NamespaceBody* namespace_body, Token* name);

  ZoneVector<Method*> methods_;

  DISALLOW_COPY_AND_ASSIGN(MethodGroup);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_METHOD_H_
