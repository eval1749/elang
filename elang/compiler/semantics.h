// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SEMANTICS_H_
#define ELANG_COMPILER_SEMANTICS_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/ir/nodes_forward.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Semantics
//
class Semantics final {
 public:
  Semantics();
  ~Semantics();

  // Returns mapping for testing.
  const std::unordered_map<ast::Node*, ir::Node*> all() const {
    return value_map_;
  }

  // Retrieving
  ir::Node* ValueOf(ast::Node* node) const;

  // Storing
  void SetValue(ast::Node* node, ir::Node* value);

 private:
  // Mapping from AST class, enum, and method to IR object
  std::unordered_map<ast::Node*, ir::Node*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(Semantics);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SEMANTICS_H_
