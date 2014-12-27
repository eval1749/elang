// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_type_and_name_h)
#define INCLUDE_elang_compiler_ast_type_and_name_h

namespace elang {
namespace compiler {
class Token;
namespace ast {

class Expression;

struct TypeAndName {
  Token* name;
  Expression* type;

  TypeAndName(Expression* type, Token* name) : name(name), type(type) {
  }
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_type_and_name_h)
