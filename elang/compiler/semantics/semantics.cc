// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/semantics/semantics.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace sm {

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

}  // namespace sm
}  // namespace compiler
}  // namespace elang
