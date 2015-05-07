// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SEMANTICS_SEMANTICS_H_
#define ELANG_COMPILER_SEMANTICS_SEMANTICS_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/semantics/nodes_forward.h"

namespace elang {
namespace compiler {
namespace sm {
class Editor;

//////////////////////////////////////////////////////////////////////
//
// Semantics
//
class Semantics final {
 public:
  Semantics();
  ~Semantics();

  // Returns mapping for testing.
  const std::unordered_map<ast::Node*, sm::Semantic*> all() const {
    return semantic_map_;
  }

  // Retrieving
  sm::Semantic* SemanticOf(ast::Node* node) const;

 private:
  friend class sm::Editor;

  // Mapping from AST class, enum, and method to IR object
  std::unordered_map<ast::Node*, sm::Semantic*> semantic_map_;

  DISALLOW_COPY_AND_ASSIGN(Semantics);
};

}  // namespace sm
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SEMANTICS_SEMANTICS_H_
