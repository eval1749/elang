// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_SHELL_NAMESPACE_BUILDER_H_
#define ELANG_SHELL_NAMESPACE_BUILDER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {

class NameResolver;
enum class PredefinedName;
class Token;
enum class TokenType;

namespace ast {
class Factory;
}

namespace ir {
class Class;
}

namespace shell {

// TODO(eval1749) |shell::NamespaceBuilder| is copy of
// |testing::NamespaceBuilder|, we should move |NamespaceBuild| to common
// place to share code.

//////////////////////////////////////////////////////////////////////
//
// NamespaceBuilder
// Builds namespace, class, method and so on.
//
class NamespaceBuilder : public CompilationSessionUser {
 public:
  explicit NamespaceBuilder(NameResolver* name_resolver);
  ~NamespaceBuilder();

  ast::Factory* ast_factory() const;
  NameResolver* name_resolver() const { return name_resolver_; }
  ir::Class* system_object();

  ast::ClassBody* NewClass(base::StringPiece name,
                           base::StringPiece base_names);
  Token* NewKeyword(TokenType type);
  Token* NewName(base::StringPiece name);
  ast::Parameter* NewParameter(ast::Method* method,
                               int position,
                               base::StringPiece type,
                               base::StringPiece name);
  ast::Type* NewTypeReference(TokenType keyword);
  ast::Type* NewTypeReference(base::StringPiece name);

 private:
  NameResolver* const name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBuilder);
};

}  // namespace shell
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_SHELL_NAMESPACE_BUILDER_H_
