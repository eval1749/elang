// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_
#define ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_

#include "base/macros.h"
#include "elang/compiler/analyze/analyzer.h"
#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace compiler {
namespace ts {
class Value;
}

class CompilationSession;
class MethodResolver;

//////////////////////////////////////////////////////////////////////
//
// TypeResolver
//
class TypeResolver final : public Analyzer, public ast::Visitor {
 public:
  TypeResolver(NameResolver* name_resolver, ast::Method* method);
  ~TypeResolver();

  // Evaluate type value of |node| for |user|.
  ts::Value* Evaluate(ast::Node* node, ast::Node* user);
  // Unify type value of |expression| with |value|.
  bool Unify(ast::Expression* expression, ts::Value* value);

 private:
  struct Context;
  class ScopedContext;

  ts::Value* EmptyValue();
  ts::Value* Intersect(ts::Value* value1, ts::Value* value2);
  ts::Value* NewInvalidValue(ast::Node* node);
  void ProduceResult(ts::Value* value, ast::Node* producer);
  ast::NamedNode* ResolveReference(ast::Expression* expression);
  ts::Value* Union(ts::Value* value1, ts::Value* value2);

  // ast::Visitor
  void VisitCall(ast::Call* call);
  void VisitLiteral(ast::Literal* literal);

  Context* context_;
  ast::Method* const method_;
  MethodResolver* const method_resolver_;

  DISALLOW_COPY_AND_ASSIGN(TypeResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_
