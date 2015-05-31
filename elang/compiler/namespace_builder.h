// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_NAMESPACE_BUILDER_H_
#define ELANG_COMPILER_NAMESPACE_BUILDER_H_

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
enum class ParameterKind;
enum class PredefinedName;
class Token;
enum class TokenType;

namespace sm {
class Class;
class Editor;
class Method;
class Parameter;
class Semantic;
}

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
  NameResolver* name_resolver() { return name_resolver_; }
  sm::Class* system_object();

  ast::ClassBody* NewClass(base::StringPiece name,
                           base::StringPiece base_names);
  ast::ClassBody* NewStruct(base::StringPiece name,
                            base::StringPiece base_names);
  Token* NewKeyword(TokenType type);
  Token* NewName(base::StringPiece name);
  sm::Parameter* NewParameter(ParameterKind kind,
                              int position,
                              base::StringPiece type,
                              base::StringPiece name);
  ast::Type* NewTypeReference(TokenType keyword);
  ast::Type* NewTypeReference(base::StringPiece name);
  sm::Semantic* SemanticOf(base::StringPiece16 name) const;
  sm::Semantic* SemanticOf(base::StringPiece name) const;

 private:
  ast::ClassBody* NewClass(TokenType token_type,
                           base::StringPiece name,
                           base::StringPiece base_names);

  std::unique_ptr<AnalysisEditor> analysis_editor_;
  NameResolver* const name_resolver_;
  std::unique_ptr<sm::Editor> semantic_editor_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBuilder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_NAMESPACE_BUILDER_H_
