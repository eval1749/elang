// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NAMED_NODE_H_
#define ELANG_COMPILER_AST_NAMED_NODE_H_

#include <vector>

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NamedNode
//
class NamedNode : public Node {
  DECLARE_CASTABLE_CLASS(NamedNode, Node);

 public:
  Token* keyword() const { return token(); }
  Token* name() const { return name_; }

 protected:
  NamedNode(Token* keyword, Token* name);
  ~NamedNode() override;

 private:
  Token* const name_;

  DISALLOW_COPY_AND_ASSIGN(NamedNode);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAMED_NODE_H_
