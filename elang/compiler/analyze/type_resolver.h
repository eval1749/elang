// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_
#define ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "elang/compiler/analyze/analyzer.h"
#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace compiler {

namespace ts {
class CallValue;
class Factory;
class Value;
}

class MethodResolver;
class TypeUnifier;
class VariableTracker;

//////////////////////////////////////////////////////////////////////
//
// TypeResolver
//
class TypeResolver final : public Analyzer, public ast::Visitor {
 public:
  // |context_method| is starting point of reference resolving.
  TypeResolver(NameResolver* name_resolver,
               ts::Factory* type_factory,
               VariableTracker* variable_tracker,
               ast::Method* context_method);
  ~TypeResolver();

  const std::vector<ts::CallValue*>& call_values() const {
    return call_values_;
  }

  ts::Factory* type_factory() const { return type_factory_; }

  // Unify type value of |expression| with |value|.
  bool Resolve(ast::Expression* expression, ts::Value* value);

 private:
  struct Context;
  class ScopedContext;

  ts::Value* GetAnyValue();
  ts::Value* GetEmptyValue();
  ts::Value* NewInvalidValue(ast::Node* node);
  ts::Value* NewLiteral(ir::Type* type);
  void ProduceResult(ts::Value* value, ast::Node* producer);
  void ProduceUnifiedResult(ts::Value* value, ast::Node* producer);
  ast::NamedNode* ResolveReference(ast::Expression* expression);
  ts::Value* Unify(ts::Value* value1, ts::Value* value2);

  // ast::Visitor
  void VisitCall(ast::Call* node);
  void VisitLiteral(ast::Literal* node);
  void VisitVariableReference(ast::VariableReference* node);

  Context* context_;
  ast::Method* const context_method_;
  std::vector<ts::CallValue*> call_values_;
  const std::unique_ptr<MethodResolver> method_resolver_;
  ts::Factory* type_factory_;
  VariableTracker* const variable_tracker_;

  DISALLOW_COPY_AND_ASSIGN(TypeResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_RESOLVER_H_
