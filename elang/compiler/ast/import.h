// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_IMPORT_H_
#define ELANG_COMPILER_AST_IMPORT_H_

#include "elang/compiler/ast/namespace_member.h"

#include "elang/compiler/qualified_name.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Import
//
class Import final : public NamespaceMember {
  DECLARE_AST_NODE_CONCRETE_CLASS(Import, NamespaceMember);

 public:
  Expression* reference() const { return reference_; }

 private:
  Import(NamespaceBody* namespace_body, Token* keyword, Expression* reference);

  Expression* const reference_;

  DISALLOW_COPY_AND_ASSIGN(Import);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_IMPORT_H_
