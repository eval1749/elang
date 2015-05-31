// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "elang/compiler/analysis/name_resolver_editor.h"

#include "base/logging.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/namespace.h"

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

void NameResolverEditor::RegisterAlias(ast::Alias* alias,
                                       ast::ContainerNode* resolved) {
  DCHECK(!resolver_->alias_map_.count(alias));
  resolver_->alias_map_.insert(std::make_pair(alias, resolved));
}

void NameResolverEditor::RegisterImport(ast::Import* import,
                                        ast::ContainerNode* resolved) {
  DCHECK(!resolver_->import_map_.count(import));
  resolver_->import_map_.insert(std::make_pair(import, resolved));
}

}  // namespace compiler
}  // namespace elang
