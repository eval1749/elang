// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_
#define ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "base/macros.h"

namespace elang {
namespace compiler {
namespace ast {
class Expression;
class ContainerNode;
class NamedNode;
}
namespace ir {
class Node;
class Factory;
}

class CompilationSession;

//////////////////////////////////////////////////////////////////////
//
// TypeResolver
//
class TypeResolver final {
 public:
  explicit TypeResolver(NameResolver* name_resolver);
  ~TypeResolver();

  ir::Method* ResolveCall(ast::Call* call);
  ir::Type* ResolveExpression(ast::Expression* expression);
  void AddExpression(ast::Expression* expression, ir::Type* result_type);
  bool Run();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;

  DISALLOW_COPY_AND_ASSIGN(TypeResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_
