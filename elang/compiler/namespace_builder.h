// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_NAMESPACE_BUILDER_H_
#define ELANG_COMPILER_NAMESPACE_BUILDER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {
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
  explicit NamespaceBuilder(NameResolver* resolver);
  ~NamespaceBuilder();

  sm::Class* system_object();

  sm::Class* NewClass(base::StringPiece name, base::StringPiece base_names);
  sm::Class* NewStruct(base::StringPiece name, base::StringPiece base_names);
  Token* NewKeyword(TokenType type);
  Token* NewName(base::StringPiece name);
  sm::Parameter* NewParameter(ParameterKind kind,
                              int position,
                              base::StringPiece type,
                              base::StringPiece name);
  sm::Semantic* SemanticOf(base::StringPiece16 name) const;
  sm::Semantic* SemanticOf(base::StringPiece name) const;

 private:
  sm::Class* NewClass(TokenType token_type,
                      base::StringPiece name,
                      base::StringPiece base_names);

  std::unique_ptr<sm::Editor> semantic_editor_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBuilder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_NAMESPACE_BUILDER_H_
