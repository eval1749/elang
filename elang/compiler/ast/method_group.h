// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_METHOD_GROUP_H_
#define ELANG_COMPILER_AST_METHOD_GROUP_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace compiler {
namespace ast {

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

  // Node
  void Accept(Visitor* visitor) override;

  ZoneVector<Method*> methods_;

  DISALLOW_COPY_AND_ASSIGN(MethodGroup);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_METHOD_GROUP_H_
