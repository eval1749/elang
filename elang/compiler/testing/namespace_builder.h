// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TESTING_NAMESPACE_BUILDER_H_
#define ELANG_COMPILER_TESTING_NAMESPACE_BUILDER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/compiler/ast/nodes_forward.h"

namespace elang {
namespace compiler {

class CompilationSession;
class NameResolver;
enum class PredefinedName;
class Token;
enum class TokenType;

namespace ir {
class Class;
}

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// NamespaceBuilder
// Builds namespace, class, method and so on.
//
class NamespaceBuilder {
 protected:
  explicit NamespaceBuilder(NameResolver* name_resolver);
  ~NamespaceBuilder();

  NameResolver* name_resolver() { return name_resolver_; }
  CompilationSession* session();
  ast::Namespace* system_namespace();
  ast::NamespaceBody* system_namespace_body();
  ir::Class* system_object();

  ast::Class* GetPredefinedType(PredefinedName name);
  ast::ClassBody* NewClass(base::StringPiece name,
                           base::StringPiece base_names);
  Token* NewKeyword(TokenType type);
  Token* NewName(base::StringPiece name);
  ast::Variable* NewParameter(base::StringPiece type, base::StringPiece name);
  ast::Type* NewTypeReference(TokenType keyword);
  ast::Type* NewTypeReference(base::StringPiece name);

 private:
  NameResolver* const name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBuilder);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TESTING_NAMESPACE_BUILDER_H_
