// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/compilation_session_user.h"

#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/ast/nodes.h"
#include "elang/compiler/compilation_session.h"

namespace elang {
namespace compiler {

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
  session()->AddError(error_code, node->name());
}

void CompilationSessionUser::Error(ErrorCode error_code, Token* token) {
  session()->AddError(error_code, token);
}

void CompilationSessionUser::Error(ErrorCode error_code,
                                   ast::Node* node,
                                   ast::Node* node2) {
  session()->AddError(error_code, node->name(), node2->name());
}

void CompilationSessionUser::Error(ErrorCode error_code,
                                   Token* token,
                                   Token* token2) {
  session()->AddError(error_code, token, token2);
}

sm::Type* CompilationSessionUser::PredefinedTypeOf(PredefinedName name) {
  return session()->PredefinedTypeOf(name);
}

}  // namespace compiler
}  // namespace elang
