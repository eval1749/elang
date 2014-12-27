// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_visitor_h)
#define INCLUDE_elang_compiler_ast_visitor_h

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Visitor
//
class Visitor  {
  public: Visitor();
  public: virtual ~Visitor();

  public: virtual void Visit(Node* node) = 0;

    #define DEF_VISIT(type) \
        public: virtual void Visit##type(type* node) = 0;
    AST_NODE_LIST(DEF_VISIT)
    #undef DEF_VISIT

  DISALLOW_COPY_AND_ASSIGN(Visitor);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_visitor_h)
