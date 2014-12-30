// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/name_resolver.h"

#include "base/logging.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// NameResolver
//
NameResolver::NameResolver(CompilationSession* session) : session_(session) {
}

NameResolver::~NameResolver() {
}

ast::NamespaceMember* NameResolver::FindReference(ast::Expression* reference) {
  auto const it = map_.find(reference);
  return it == map_.end() ? nullptr : it->second;
}

void NameResolver::Resolved(ast::Expression* reference,
                            ast::NamespaceMember* member) {
  DCHECK(member);
  DCHECK(map_.find(reference) == map_.end());
  map_[reference] = member;
}

}  // namespace compiler
}  // namespace elang
