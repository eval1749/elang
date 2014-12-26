// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_enum_h)
#define INCLUDE_elang_compiler_ast_enum_h

#include <unordered_map>
#include <vector>

#include "elang/compiler/ast/namespace_member.h"

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {
namespace ast {

class Class;
class EnumMember;
class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Enum
//
class Enum final : public NamespaceMember {
  DECLARE_CASTABLE_CLASS(Enum, NamespaceMember);

  friend class NodeFactory;

  private: std::unordered_map<hir::SimpleName*, EnumMember*> map_;
  private: std::vector<EnumMember*> members_;

  public: Enum(NamespaceBody* namespace_body, Token* keyword,
               Token* simple_name);
  public: ~Enum() final;

  public: void AddMember(EnumMember* member);
  public: EnumMember* FindMember(Token* simple_name);

  DISALLOW_COPY_AND_ASSIGN(Enum);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_enum_h)

