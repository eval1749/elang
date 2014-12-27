// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_name_reference_h)
#define INCLUDE_elang_compiler_ast_name_reference_h

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
  DECLARE_CASTABLE_CLASS(NameReference, Expression);

  friend class NodeFactory;

  private: NameReference(Token* name);
  public: ~NameReference() final;

  DISALLOW_COPY_AND_ASSIGN(NameReference);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_name_reference_h)

