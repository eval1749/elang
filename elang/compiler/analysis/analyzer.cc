// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/analyzer.h"

#include "base/logging.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/semantics.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Analyzer
//
Analyzer::Analyzer(NameResolver* name_resolver)
    : CompilationSessionUser(name_resolver->session()),
      editor_(new sm::Editor(session(), name_resolver->factory())),
      name_resolver_(name_resolver) {
}

Analyzer::~Analyzer() {
}

sm::Factory* Analyzer::ir_factory() const {
  return name_resolver_->factory();
}

void Analyzer::FixSemanticOf(ast::Node* node, sm::Semantic* semantic) {
  DCHECK(!semantics()->SemanticOf(node));
  editor()->SetSemanticOf(node, semantic);
}

sm::Semantic* Analyzer::Resolve(ast::NamedNode* ast_node) {
  return name_resolver_->Resolve(ast_node);
}

sm::Type* Analyzer::ResolveTypeReference(ast::Type* type,
                                         ast::ContainerNode* container) {
  if (auto const array_type = type->as<ast::ArrayType>()) {
    auto const element_type =
        ResolveTypeReference(array_type->element_type(), container);
    std::vector<int> dimensions(array_type->dimensions().begin(),
                                array_type->dimensions().end());
    auto const value = factory()->NewArrayType(element_type, dimensions);
    SetSemanticOf(type, value);
    return value;
  }
  auto const ast_node = name_resolver_->ResolveReference(type, container);
  if (!ast_node) {
    DVLOG(0) << "Type not found: " << *type << " in " << *container
             << std::endl;
    Error(ErrorCode::AnalyzeTypeNotFound, type);
    return nullptr;
  }
  return Resolve(ast_node)->as<sm::Type>();
}

void Analyzer::SetSemanticOf(ast::Node* node, sm::Semantic* semantic) {
  editor()->SetSemanticOf(node, semantic);
}

}  // namespace compiler
}  // namespace elang
