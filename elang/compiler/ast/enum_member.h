// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(elang_compiler_ast_enum_member_h)
#define elang_compiler_ast_enum_member_h

#include <unordered_map>

#include "elang/compiler/ast/node.h"

namespace elang {
namespace compiler {
namespace ast {

class Enum;
class Expression;
class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// EnumMember
//
class EnumMember final : public Node {
  DECLARE_CASTABLE_CLASS(EnumMember, Node);

  friend class NodeFactory;

  private: Expression* expression_;

  private: EnumMember(Enum* owner, const Token& simple_name,
                      Expression* expression);
  public: ~EnumMember() final;

  public: const Token& simple_name() const { return token(); }

  DISALLOW_COPY_AND_ASSIGN(EnumMember);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(elang_compiler_ast_enum_member_h)

