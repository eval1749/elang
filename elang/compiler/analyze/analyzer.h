// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_ANALYZER_H_
#define ELANG_COMPILER_ANALYZE_ANALYZER_H_

#include <unordered_map>

#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace compiler {

namespace ir {
class Factory;
class ContainerNode;
class Node;
}

class CompilationSession;
enum class ErrorCode;
class NameResolver;
class Signature;

//////////////////////////////////////////////////////////////////////
//
// Analyzer
//
class Analyzer {
 protected:
  explicit Analyzer(NameResolver* resolver);
  virtual ~Analyzer();

  ir::Factory* factory() const;
  NameResolver* resolver() const { return resolver_; }
  CompilationSession* session() const;

  // Report error caused by |node|.
  void Error(ErrorCode error_code, ast::Node* node);
  void Error(ErrorCode error_code, ast::Node* node, ast::Node* node2);

  // Shortcut to |NameResolver|.
  ir::Node* Resolve(ast::NamedNode* ast_node);
  ir::Node* ResolveTypeReference(ast::Expression* reference,
                                 ast::ContainerNode* container);

 private:
  NameResolver* const resolver_;

  DISALLOW_COPY_AND_ASSIGN(Analyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_ANALYZER_H_
