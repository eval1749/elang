// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/semantics.h"

#include "base/logging.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Semantics
//
Semantics::Semantics() {
}

Semantics::~Semantics() {
}

// It is valid to pass |nullptr| to |node| for avoiding null check in call
// site, see |TypeEvaluator::VisitLiteral()| as example.
sm::Semantic* Semantics::SemanticOf(ast::Node* node) const {
  auto const it = semantic_map_.find(node);
  return it == semantic_map_.end() ? nullptr : it->second;
}

void Semantics::SetSemanticOf(ast::Node* node, sm::Semantic* semantic) {
  DCHECK(node);
  DCHECK(semantic);
  semantic_map_[node] = semantic;
}

}  // namespace compiler
}  // namespace elang
