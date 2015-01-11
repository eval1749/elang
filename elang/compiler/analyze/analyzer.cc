// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/analyzer.h"

#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Analyzer
//
Analyzer::Analyzer(NameResolver* resolver) : resolver_(resolver) {
}

Analyzer::~Analyzer() {
}

ir::Factory* Analyzer::factory() const {
  return resolver_->factory();
}

CompilationSession* Analyzer::session() const {
  return resolver_->session();
}

void Analyzer::Error(ErrorCode error_code, ast::Node* node) {
  session()->AddError(error_code, node->name());
}

void Analyzer::Error(ErrorCode error_code, ast::Node* node, ast::Node* node2) {
  session()->AddError(error_code, node->name(), node2->name());
}

ir::Node* Analyzer::Resolve(ast::NamedNode* ast_node) {
  return resolver_->Resolve(ast_node);
}

ir::Node* Analyzer::ResolveTypeReference(ast::Expression* reference,
                                         ast::ContainerNode* container) {
  auto const ast_node = resolver_->ResolveReference(reference, container);
  if (!ast_node) {
    Error(ErrorCode::AnalyzeTypeNotFound, reference);
    return nullptr;
  }
  if (!ast_node->is_type())
    return nullptr;
  return Resolve(ast_node);
}

}  // namespace compiler
}  // namespace elang
