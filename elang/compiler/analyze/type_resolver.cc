// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/type_resolver.h"

#include "base/logging.h"
#include "elang/compiler/analyze/method_resolver.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/analyze/type_evaluator.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// TypeResolver::Context
//
struct TypeResolver::Context {
  ts::Value* result;
  ts::Value* value;
  ast::Node* user;

  Context(ts::Value* value, ast::Node* user)
      : result(nullptr), user(user), value(value) {}
};

//////////////////////////////////////////////////////////////////////
//
// TypeResolver::ScopedContext
//
class TypeResolver::ScopedContext {
 public:
  ScopedContext(TypeResolver* resolver, ts::Value* value, ast::Node* user);
  ~ScopedContext();

 private:
  Context context_;
  TypeResolver* const resolver_;
  Context* const saved_context_;

  DISALLOW_COPY_AND_ASSIGN(ScopedContext);
};

TypeResolver::ScopedContext::ScopedContext(TypeResolver* resolver,
                                           ts::Value* value,
                                           ast::Node* user)
    : context_(value, user),
      resolver_(resolver),
      saved_context_(resolver->context_) {
  resolver_->context_ = &context_;
}

TypeResolver::ScopedContext::~ScopedContext() {
  resolver_->context_ = saved_context_;
}

//////////////////////////////////////////////////////////////////////
//
// TypeResolver
//
TypeResolver::TypeResolver(NameResolver* name_resolver, ast::Method* method)
    : Analyzer(name_resolver),
      context_(nullptr),
      method_(method),
      method_resolver_(new MethodResolver()),
      type_evaluator_(new TypeEvaluator(name_resolver)) {
}

TypeResolver::~TypeResolver() {
}

ts::Value* TypeResolver::Evaluate(ast::Node* node, ast::Node* user) {
  return type_evaluator_->Evaluate(node, user);
}

ts::Value* TypeResolver::GetEmptyValue() {
  return type_evaluator_->GetEmptyValue();
}

ts::Value* TypeResolver::Intersect(ts::Value* value1, ts::Value* value2) {
  return type_evaluator_->Intersect(value1, value2);
}

ts::Value* TypeResolver::NewInvalidValue(ast::Node* node) {
  return type_evaluator_->NewInvalidValue(node);
}

void TypeResolver::ProduceResult(ts::Value* result, ast::Node* producer) {
  DCHECK(result);
  DCHECK(context_);
  DCHECK(!context_->result);
  DCHECK(producer);
  context_->result = result;
  // TODO(eval1749) If |result| is |EmptyValue|, report error with |producer|.
}

ast::NamedNode* TypeResolver::ResolveReference(ast::Expression* expression) {
  return resolver()->ResolveReference(expression, method_);
}

bool TypeResolver::Unify(ast::Expression* expression, ts::Value* value) {
  ScopedContext context(this, value, expression);
  expression->Accept(this);
  // TODO(eval1749) Returns false if |context_.result| is |EmptyType|.
  return true;
}

ts::Value* TypeResolver::Union(ts::Value* value1, ts::Value* value2) {
  return type_evaluator_->Union(value1, value2);
}

// ats::Visitor
void TypeResolver::VisitCall(ast::Call* call) {
  std::vector<ts::Value*> arguments;
  for (auto const argument : call->arguments())
    arguments.push_back(Evaluate(argument, call));
  auto const callee = ResolveReference(call->callee());
  auto const method_group = callee->as<ast::MethodGroup>();
  if (!method_group) {
    // TODO(eval1749) NYI call site other than method call.
    Error(ErrorCode::TypeResolverCalleeNotSupported, call->callee());
    ProduceResult(NewInvalidValue(call->callee()), call);
    return;
  }
  // Compute matching methods.
  auto const methods =
      method_resolver_->Resolve(method_group, context_->value, arguments);
  if (methods.empty()) {
    Error(ErrorCode::TypeResolverMethodNoMatch, call);
    ProduceResult(NewInvalidValue(call->callee()), call);
    return;
  }
  if (methods.size() == 1) {
    // The value of this call site is determined. We compute argument values
    // and return value and propagate them to users.
    auto const method = *methods.begin();
    auto parameters = method->parameters().begin();
    auto succeeded = true;
    for (auto const argument : call->arguments()) {
      // Propagate argument value to users.
      if (!Unify(argument, Evaluate(*parameters, call))) {
        Error(ErrorCode::TypeResolverArgumentUnify, argument);
        succeeded = false;
      }
      ++parameters;
    }
    if (!succeeded) {
      ProduceResult(NewInvalidValue(call->callee()), call);
      return;
    }
    // TODO(eval1749) Update users of this call site.
    ProduceResult(Intersect(Evaluate(method->return_type(), context_->user),
                            context_->value),
                  call);
    return;
  }
  // The result value is union of return value of candidate methods.
  auto result = GetEmptyValue();
  for (auto const method : methods)
    result = Union(Evaluate(method->return_type(), context_->user), result);
  ProduceResult(Intersect(result, context_->value), call);
}

void TypeResolver::VisitLiteral(ast::Literal* literal) {
  auto const literal_value = Evaluate(literal, literal);
  ProduceResult(Intersect(literal_value, context_->value), literal);
}

}  // namespace compiler
}  // namespace elang
