// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_TYPE_RESOLVER_H_
#define ELANG_COMPILER_ANALYSIS_TYPE_RESOLVER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "elang/compiler/analysis/analyzer.h"
#include "elang/compiler/analysis/type_factory_user.h"
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
class TypeResolver final : public Analyzer,
                           public ast::Visitor,
                           public ts::FactoryUser {
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

  // Unify type value of |expression| with |value|.
  ts::Value* Resolve(ast::Expression* expression, ts::Value* value);

  // Returns bool value if |expression| is boolean expression, otherwise
  // empty value.
  ts::Value* ResolveAsBool(ast::Expression* expression);

 private:
  struct Context;
  class ScopedContext;

  struct NumericType {
    enum class Kind { Float, Int, None, UInt };

    Kind kind;
    int size;

    NumericType(Kind kind, int size) : kind(kind), size(size) {}
  };

  NumericType NumericTypeOf(ts::Value* vlaue) const;

  ts::Value* PromoteNumericType(NumericType left_type,
                                NumericType right_type) const;

  void ProduceResolved(ast::Expression* expression,
                       ts::Value* value,
                       ast::Node* produce);
  void ProduceResult(ts::Value* value, ast::Node* producer);
  void ProduceUnifiedResult(ts::Value* value, ast::Node* producer);
  void ProduceSemantics(ts::Value* value, ast::Node* node);

  ast::NamedNode* ResolveReference(ast::Expression* expression);
  ts::Value* Unify(ts::Value* value1, ts::Value* value2);

  // Shortcut function.
  ir::Node* ValueOf(ast::Node* node);

  // ast::Visitor
  void VisitAssignment(ast::Assignment* node) final;
  void VisitBinaryOperation(ast::BinaryOperation* node) final;
  void VisitCall(ast::Call* node) final;
  void VisitConditional(ast::Conditional* node) final;
  void VisitLiteral(ast::Literal* node) final;
  void VisitParameterReference(ast::ParameterReference* node) final;
  void VisitVariableReference(ast::VariableReference* node) final;

  Context* context_;
  ast::Method* const context_method_;
  std::vector<ts::CallValue*> call_values_;
  const std::unique_ptr<MethodResolver> method_resolver_;
  VariableTracker* const variable_tracker_;

  DISALLOW_COPY_AND_ASSIGN(TypeResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_TYPE_RESOLVER_H_
