// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_
#define ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "base/macros.h"

namespace elang {
namespace compiler {
namespace ast {
class NamedNode;
}
namespace ir {
class Node;
class Factory;
}

class CompilationSession;

//////////////////////////////////////////////////////////////////////
//
// NameResolver
// Keeps analysis results from |NamespaceAnalyzer| for mapping name reference
// to |ast::NamedNode|.
//
class NameResolver final {
 public:
  explicit NameResolver(CompilationSession* session);
  ~NameResolver();

  ir::Factory* factory() const { return factory_.get(); }
  CompilationSession* session() const { return session_; }

  void DidResolve(ast::NamedNode* ast_node, ir::Node* node);
  ir::Node* Resolve(ast::NamedNode* ast_node) const;

 private:
  const std::unique_ptr<ir::Factory> factory_;
  std::unordered_map<ast::NamedNode*, ir::Node*> node_map_;
  CompilationSession* const session_;

  DISALLOW_COPY_AND_ASSIGN(NameResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_
