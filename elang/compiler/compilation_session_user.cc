// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/compilation_session_user.h"

#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics.h"

namespace elang {
namespace compiler {

CompilationSessionUser::CompilationSessionUser(CompilationSession* session)
    : session_(session) {
}

CompilationSessionUser::~CompilationSessionUser() {
}

Semantics* CompilationSessionUser::semantics() const {
  return session()->semantics();
}

ast::Namespace* CompilationSessionUser::system_namespace() {
  return session()->system_namespace();
}

ast::NamespaceBody* CompilationSessionUser::system_namespace_body() {
  return session()->system_namespace_body();
}

ast::Class* CompilationSessionUser::GetPredefinedType(PredefinedName name) {
  return session()->GetPredefinedType(name);
}

}  // namespace compiler
}  // namespace elang
