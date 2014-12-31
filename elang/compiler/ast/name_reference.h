// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NAME_REFERENCE_H_
#define ELANG_COMPILER_AST_NAME_REFERENCE_H_

#include "elang/compiler/ast/expression.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// NameReference
//
class NameReference final : public Expression {
  DECLARE_AST_NODE_CLASS(NameReference, Expression);

 public:
  Token* name() const { return token(); }

 private:
  explicit NameReference(Token* name);

  // Node
  void Accept(Visitor* visitor) override;

  DISALLOW_COPY_AND_ASSIGN(NameReference);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAME_REFERENCE_H_
