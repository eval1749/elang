// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_STATEMENT_H_
#define ELANG_COMPILER_AST_STATEMENT_H_

#include <vector>

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Statement
//
class Statement : public Node {
  DECLARE_CASTABLE_CLASS(Statement, Node);

  friend class NodeFactory;

 public:
  ~Statement() override;

  Token* op() const { return token(); }

 protected:
  explicit Statement(Token* op);

 private:
  DISALLOW_COPY_AND_ASSIGN(Statement);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_STATEMENT_H_
