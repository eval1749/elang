// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_CATCH_CLAUSE_H_
#define ELANG_COMPILER_AST_CATCH_CLAUSE_H_

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// CatchClause
//
class CatchClause final : public Node {
  DECLARE_CASTABLE_CLASS(CatchClause, Node);
  friend class NodeFactory;

 public:
  BlockStatement* block() const { return block_; }
  Expression* type() const { return type_; }
  LocalVariable* variable() const { return variable_; }

 private:
  CatchClause(Token* keyword,
              Expression* type,
              LocalVariable* variable,
              BlockStatement* block);
  ~CatchClause() final;

  BlockStatement* const block_;
  Expression* const type_;
  LocalVariable* const variable_;

  DISALLOW_COPY_AND_ASSIGN(CatchClause);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_CATCH_CLAUSE_H_
