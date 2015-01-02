// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_VISITOR_H_
#define ELANG_COMPILER_AST_VISITOR_H_

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Visitor
//
class Visitor {
 public:
  Visitor();
  virtual ~Visitor();

  virtual void Visit(Node* node) = 0;

#define DEF_VISIT(type) virtual void Visit##type(type* node) = 0;
  FOR_EACH_AST_NODE(DEF_VISIT)
#undef DEF_VISIT

 private:
  DISALLOW_COPY_AND_ASSIGN(Visitor);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_VISITOR_H_
