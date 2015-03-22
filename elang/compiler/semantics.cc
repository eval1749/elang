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

void Semantics::SetValue(ast::Node* node, sm::Node* value) {
  DCHECK(node);
  value_map_[node] = value;
}

// It is valid to pass |nullptr| to |node| for avoiding null check in call
// site, see |TypeEvaluator::VisitLiteral()| as example.
sm::Node* Semantics::ValueOf(ast::Node* node) const {
  auto const it = value_map_.find(node);
  return it == value_map_.end() ? nullptr : it->second;
}

}  // namespace compiler
}  // namespace elang
