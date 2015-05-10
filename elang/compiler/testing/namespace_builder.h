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
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {
class AnalysisEditor;
class NameResolver;
enum class PredefinedName;
class Token;
enum class TokenType;

namespace sm {
class Class;
}

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// NamespaceBuilder
// Builds namespace, class, method and so on.
//
class NamespaceBuilder : public CompilationSessionUser {
 public:
  explicit NamespaceBuilder(NameResolver* name_resolver);
  ~NamespaceBuilder();

  NameResolver* name_resolver() { return name_resolver_; }
  sm::Class* system_object();

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
  std::unique_ptr<AnalysisEditor> analysis_editor_;
  NameResolver* const name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBuilder);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TESTING_NAMESPACE_BUILDER_H_
