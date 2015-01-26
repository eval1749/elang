// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/analyzer.h"

#include "base/logging.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Analyzer
//
Analyzer::Analyzer(NameResolver* name_resolver)
    : CompilationSessionUser(name_resolver->session()),
      name_resolver_(name_resolver) {
}

Analyzer::~Analyzer() {
}

ir::Factory* Analyzer::ir_factory() const {
  return name_resolver_->factory();
}

ir::Node* Analyzer::Resolve(ast::NamedNode* ast_node) {
  return name_resolver_->Resolve(ast_node);
}

ir::Type* Analyzer::ResolveTypeReference(ast::Type* type,
                                         ast::ContainerNode* container) {
  auto const ast_node = name_resolver_->ResolveReference(type, container);
  if (!ast_node) {
    DVLOG(0) << "Type not found: " << *type << " in " << *container
             << std::endl;
    Error(ErrorCode::AnalyzeTypeNotFound, type);
    return nullptr;
  }
  return Resolve(ast_node)->as<ir::Type>();
}

}  // namespace compiler
}  // namespace elang
