// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/compiler/compilation_session_user.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {
bool ShouldUsePrinter(ast::Node* node) {
  return node->is<ast::MemberAccess>() || node->is<ast::TypeMemberAccess>();
}
}  // namespace

CompilationSessionUser::CompilationSessionUser(CompilationSession* session)
    : session_(session) {
}

CompilationSessionUser::~CompilationSessionUser() {
}

Analysis* CompilationSessionUser::analysis() const {
  return session()->analysis();
}

ast::Namespace* CompilationSessionUser::system_namespace() {
  return session()->system_namespace();
}

ast::NamespaceBody* CompilationSessionUser::system_namespace_body() {
  return session()->system_namespace_body();
}

void CompilationSessionUser::Error(ErrorCode error_code, ast::Node* node) {
  session()->AddError(error_code, PrettyTokenFor(node));
}

void CompilationSessionUser::Error(ErrorCode error_code, Token* token) {
  session()->AddError(error_code, token);
}

void CompilationSessionUser::Error(ErrorCode error_code,
                                   ast::Node* node,
                                   ast::Node* node2) {
  session()->AddError(error_code, PrettyTokenFor(node), PrettyTokenFor(node2));
}

void CompilationSessionUser::Error(ErrorCode error_code,
                                   ast::Node* node,
                                   Token* token) {
  session()->AddError(error_code, PrettyTokenFor(node), token);
}

void CompilationSessionUser::Error(ErrorCode error_code,
                                   Token* token,
                                   Token* token2) {
  session()->AddError(error_code, token, token2);
}

sm::Type* CompilationSessionUser::PredefinedTypeOf(PredefinedName name) {
  return session()->PredefinedTypeOf(name);
}

Token* CompilationSessionUser::PrettyTokenFor(ast::Node* node) {
  if (!ShouldUsePrinter(node))
    return node->name();
  std::stringstream ostream;
  ostream << node;
  auto const name =
      session()->NewAtomicString(base::UTF8ToUTF16(ostream.str()));
  return session()->NewToken(node->token()->location(),
                             TokenData(TokenType::SimpleName, name));
}

}  // namespace compiler
}  // namespace elang
