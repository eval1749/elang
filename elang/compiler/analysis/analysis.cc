// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/analysis.h"

#include "base/logging.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Analysis
//
Analysis::Analysis() {
}

Analysis::~Analysis() {
}

// It is valid to pass |nullptr| to |node| for avoiding null check in call
// site, see |TypeEvaluator::VisitLiteral()| as example.
sm::Semantic* Analysis::SemanticOf(ast::Node* node) const {
  auto const it = semantic_map_.find(node);
  return it == semantic_map_.end() ? nullptr : it->second;
}

}  // namespace compiler
}  // namespace elang
