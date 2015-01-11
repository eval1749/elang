// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/name_resolver.h"

#include "base/logging.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// NameResolver
//
NameResolver::NameResolver(CompilationSession* session)
    : factory_(new ir::Factory()), session_(session) {
}

NameResolver::~NameResolver() {
}

void NameResolver::DidResolve(ast::NamedNode* ast_node, ir::Node* node) {
  DCHECK(ast_node);
  DCHECK(!node_map_.count(ast_node));
  node_map_[ast_node] = node;
}

ir::Node* NameResolver::Resolve(ast::NamedNode* member) const {
  auto const it = node_map_.find(member);
  return it == node_map_.end() ? nullptr : it->second;
}

}  // namespace compiler
}  // namespace elang
