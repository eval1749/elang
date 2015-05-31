// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "elang/compiler/analysis/name_resolver_editor.h"

#include "base/logging.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// NameResolverEditor
//
NameResolverEditor::NameResolverEditor(NameResolver* resolver)
    : resolver_(resolver) {
}

NameResolverEditor::~NameResolverEditor() {
}

void NameResolverEditor::FindWithImports(
    Token* name,
    ast::NamespaceBody* ns_body,
    std::unordered_set<sm::Semantic*>* founds) {
  return resolver_->FindWithImports(name, ns_body, founds);
}

void NameResolverEditor::RegisterAlias(ast::Alias* alias,
                                       ast::ContainerNode* resolved) {
  RegisterAlias(alias, resolver_->SemanticOf(resolved));
}

void NameResolverEditor::RegisterAlias(ast::Alias* alias,
                                       sm::Semantic* resolved) {
  DCHECK(!resolved || resolved->is<sm::Class>() ||
         resolved->is<sm::Namespace>());
  DCHECK(!resolver_->alias_map_.count(alias));
  resolver_->alias_map_.insert(std::make_pair(alias, resolved));
}

void NameResolverEditor::RegisterImport(ast::Import* import,
                                        ast::ContainerNode* resolved) {
  RegisterImport(import, resolver_->SemanticOf(resolved)->as<sm::Namespace>());
}

void NameResolverEditor::RegisterImport(ast::Import* import,
                                        sm::Namespace* resolved) {
  DCHECK(!resolver_->import_map_.count(import));
  resolver_->import_map_.insert(std::make_pair(import, resolved));
}

}  // namespace compiler
}  // namespace elang
