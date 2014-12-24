// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_alias_h)
#define INCLUDE_elang_compiler_ast_alias_h

#include "elang/compiler/ast/namespace_member.h"

#include "elang/compiler/qualified_name.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Alias
//
class Alias final : public NamespaceMember {
  DECLARE_CASTABLE_CLASS(Alias, NamespaceMember);

  friend class NodeFactory;

  private: const QualifiedName target_name_;

  private: Alias(Namespace* outer, const Token& keyword,
                 const Token& simple_name,
                 const QualifiedName& reference_name);
  public: ~Alias() final;

  public: const QualifiedName& target_name() const { return target_name_; }

  DISALLOW_COPY_AND_ASSIGN(Alias);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_alias_h)

