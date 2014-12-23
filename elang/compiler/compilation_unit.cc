// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/compilation_unit.h"

#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {

ast::Namespace* CreateGlobalNamespace(CompilationSession* session,
                                      SourceCode* source_code) {
  return session->ast_factory()->NewNamespace(nullptr,
      Token(SourceCodeRange(source_code, 0, 0), TokenType::Namespace,
            session->GetOrNewAtomicString(L"namespace")),
      QualifiedName(Token(SourceCodeRange(source_code, 0, 0 ),
                          TokenType::SimpleName,
                          session->GetOrNewAtomicString(L"::"))));

}

}  // namespace

CompilationUnit::CompilationUnit(CompilationSession* session,
                                 SourceCode* source_code)
    : global_namespace_(CreateGlobalNamespace(session, source_code)), 
      source_code_(source_code) {
}

CompilationUnit::~CompilationUnit() {
}

}  // namespace compiler
}  // namespace elang
