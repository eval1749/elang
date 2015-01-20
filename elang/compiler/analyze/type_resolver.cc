// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/type_resolver.h"

#include "base/logging.h"
#include "elang/compiler/analyze/method_resolver.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_unifyer.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/token_type.h"

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
      method_resolver_(new MethodResolver(name_resolver)),
      type_factory_(new ts::Factory()) {
}

TypeResolver::~TypeResolver() {
}

bool TypeResolver::Add(ast::Expression* expression) {
  return Unify(expression, GetAnyValue());
}

ts::Value* TypeResolver::GetAnyValue() {
  return type_factory()->GetAnyValue();
}

ts::Value* TypeResolver::GetEmptyValue() {
  return type_factory()->GetEmptyValue();
}

ts::Value* TypeResolver::Intersect(ts::Value* value1, ts::Value* value2) {
  ts::TypeUnifyer unifyer(type_factory());
  return unifyer.Unify(value1, value2);
}

ts::Value* TypeResolver::NewInvalidValue(ast::Node* node) {
  return type_factory()->NewInvalidValue(node);
}

ts::Value* TypeResolver::NewLiteral(ir::Type* type) {
  return type_factory()->NewLiteral(type);
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

// The entry point of |TypeResolver|.
bool TypeResolver::Unify(ast::Expression* expression, ts::Value* value) {
  ScopedContext context(this, value, expression);
  expression->Accept(this);
  // TODO(eval1749) Returns false if |context_.result| is |EmptyType|.
  return true;
}

// ats::Visitor

// Bind applicable methods to |call->callee|.
void TypeResolver::VisitCall(ast::Call* call) {
  auto const callee = ResolveReference(call->callee());
  if (!callee)
    return;
  auto const method_group = callee->as<ast::MethodGroup>();
  if (!method_group) {
    // TODO(eval1749) NYI call site other than method call.
    Error(ErrorCode::TypeResolverCalleeNotSupported, call->callee());
    ProduceResult(NewInvalidValue(call->callee()), call);
    return;
  }

  auto const methods = method_resolver_->ComputeApplicableMethods(
      method_group, context_->value, call->arity());

  auto const call_value = type_factory()->NewCallValue(call);
  call_value->SetMethods(methods);
  call_values_.push_back(call_value);

  if (methods.empty()) {
    DVLOG(0) << "No matching methods for " << *call;
    Error(ErrorCode::TypeResolverMethodNoMatch, call);
    ProduceResult(NewInvalidValue(call->callee()), call);
    return;
  }

  if (methods.size() == 1u) {
    // We have only one applicable method. Let's check we can really call it.
    auto const method = methods.front();
    auto parameters = method->parameters().begin();
    for (auto const argument : call->arguments()) {
      auto const parameter = *parameters;
      if (!Unify(argument, NewLiteral(parameter->type()))) {
        DVLOG(0) << "Argument[" << parameter->position() << "] " << *argument
                 << " doesn't match with " << *method;
        ProduceResult(GetEmptyValue(), call);
        return;
      }
      if (!parameter->is_rest())
        ++parameters;
    }
    ProduceResult(NewLiteral(method->return_type()), call);
    return;
  }

  // We have multiple applicable methods.
  auto position = 0;
  for (auto const argument : call->arguments()) {
    if (!Unify(argument, type_factory()->NewArgument(call_value, position))) {
      DVLOG(0) << "argument[" << position
               << "] should be subtype: " << *argument;
    }
    ++position;
  }
  ProduceResult(call_value, call);
}

// `null` => |NullValue(context_->value)|
// others => |LiteralValue(type of literal data)|
void TypeResolver::VisitLiteral(ast::Literal* ast_literal) {
  auto const token = ast_literal->token();
  if (token == TokenType::NullLiteral) {
    // TODO(eval1749) We should check |context_->value| is nullable.
    ProduceResult(type_factory()->NewNullValue(context_->value), ast_literal);
    return;
  }

  // Other than |null| literal, the type of literal is predefined.
  auto const ast_type = session()->GetPredefinedType(token->literal_type());
  auto const literal_type = semantics()->ValueOf(ast_type)->as<ir::Type>();
  if (!literal_type) {
    // Predefined type isn't defined.
    ProduceResult(NewInvalidValue(ast_literal), ast_literal);
    return;
  }
  auto const result = Intersect(NewLiteral(literal_type), context_->value);
  auto const result_literal = result->as<ts::Literal>();
  if (!result_literal)
    return ProduceResult(NewInvalidValue(ast_literal), ast_literal);
  DCHECK(!semantics()->ValueOf(ast_literal));
  semantics()->SetValue(
      ast_literal,
      factory()->NewLiteral(result_literal->value(), ast_literal->token()));
  ProduceResult(result_literal, ast_literal);
}

}  // namespace compiler
}  // namespace elang
