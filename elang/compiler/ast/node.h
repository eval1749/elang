// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_node_h)
#define INCLUDE_elang_compiler_ast_node_h

#include <memory>

#include "elang/base/castable.h"
#include "elang/base/types.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable {
  DECLARE_CASTABLE_CLASS(Node, Castable);

  private: Token* const token_;

  protected: Node(Token* token);
  public: virtual ~Node();

  public: Token* token() const { return token_; }

  DISALLOW_COPY_AND_ASSIGN(Node);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_node_h)

