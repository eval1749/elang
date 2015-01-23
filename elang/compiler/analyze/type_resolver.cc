// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/type_resolver.h"

#include "base/logging.h"
#include "elang/compiler/analyze/method_resolver.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/analyze/type_evaluator.h"
#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/analyze/variable_tracker.h"
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
TypeResolver::TypeResolver(NameResolver* name_resolver,
                           ts::Factory* type_factory,
                           VariableTracker* variable_tracker,
                           ast::Method* context_method)
    : Analyzer(name_resolver),
      ts::FactoryUser(type_factory),
      context_(nullptr),
      context_method_(context_method),
      method_resolver_(new MethodResolver(name_resolver)),
      variable_tracker_(variable_tracker) {
}

TypeResolver::~TypeResolver() {
}

ts::Value* TypeResolver::Unify(ts::Value* value1, ts::Value* value2) {
  ts::Evaluator evaluator(type_factory());
  auto const result = evaluator.Unify(value1, value2);
  if (result == empty_value()) {
    DVLOG(0) << "Unify(" << *value1 << ", " << *value2 << ") yields empty.";
  }
  return result;
}

void TypeResolver::ProduceResolved(ast::Expression* expression,
                                   ts::Value* value,
                                   ast::Node* producer) {
  Resolve(expression, value);
  ProduceUnifiedResult(value, producer);
}

void TypeResolver::ProduceResult(ts::Value* result, ast::Node* producer) {
  DCHECK(result);
  DCHECK(context_);
  DCHECK(!context_->result);
  DCHECK(producer);
  context_->result = result;
  // TODO(eval1749) If |result| is |EmptyValue|, report error with |producer|.
}

// Set unified value as result.
void TypeResolver::ProduceUnifiedResult(ts::Value* result,
                                        ast::Node* producer) {
  return ProduceResult(Unify(result, context_->value), producer);
}

// The entry point of |TypeResolver|.
bool TypeResolver::Resolve(ast::Expression* expression, ts::Value* value) {
  ScopedContext context(this, value, expression);
  expression->Accept(this);
  // TODO(eval1749) Returns false if |context_.result| is |EmptyType|.
  return true;
}

ast::NamedNode* TypeResolver::ResolveReference(ast::Expression* expression) {
  return name_resolver()->ResolveReference(expression, context_method_);
}

ir::Node* TypeResolver::ValueOf(ast::Node* node) {
  return semantics()->ValueOf(node);
}

// ast::Visitor

void TypeResolver::VisitAssignment(ast::Assignment* assignment) {
  auto const lhs = assignment->left();
  auto const rhs = assignment->right();
  if (auto const reference = lhs->as<ast::ParameterReference>()) {
    auto const value = variable_tracker_->RecordSet(reference->parameter());
    ProduceResolved(rhs, value, assignment);
    return;
  }
  if (auto const reference = lhs->as<ast::VariableReference>()) {
    auto const value = variable_tracker_->RecordSet(reference->variable());
    ProduceResolved(rhs, value, assignment);
    return;
  }
  if (auto const reference = lhs->as<ast::NameReference>()) {
    auto const value = ValueOf(ResolveReference(reference));
    DCHECK(value);
    DVLOG(0) << "NYI Assign to field " << *lhs;
    return;
  }
  if (auto const reference = lhs->as<ast::MemberAccess>()) {
    auto const value = ValueOf(ResolveReference(reference));
    DCHECK(value);
    DVLOG(0) << "NYI Assign to field " << *lhs;
    return;
  }
  if (auto const reference = lhs->as<ast::ArrayAccess>()) {
    // TODO(eval1749) We need to have |ir::ArrayType| for resolving
    // |reference->array()|.
    DVLOG(0) << "NYI Assign to array " << *lhs;
    return;
  }
  Error(ErrorCode::TypeResolverAssignmentLeftValue, lhs);
}

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

  auto const candidates = method_resolver_->ComputeApplicableMethods(
      method_group, context_->value, call->arity());

  auto const call_value = type_factory()->NewCallValue(call);
  call_value->SetMethods(candidates);
  call_values_.push_back(call_value);

  if (candidates.size() == 1u) {
    // We have only one candidate method. Let's check we can really call it.
    auto const method = call_value->methods().front();
    auto parameters = method->parameters().begin();
    for (auto const argument : call->arguments()) {
      auto const parameter = *parameters;
      if (!Resolve(argument, NewLiteral(parameter->type()))) {
        DVLOG(0) << "Argument[" << parameter->position() << "] " << *argument
                 << " doesn't match with " << *method;
        call_value->SetMethods({});
        ProduceResult(empty_value(), call);
        return;
      }
      if (!parameter->is_rest())
        ++parameters;
    }
    ProduceResult(NewLiteral(method->return_type()), call);
    return;
  }

  // TODO(eval1749) Can we return literal value if all return types are same?
  if (candidates.size() >= 2u) {
    // We have multiple candidates.
    auto position = 0;
    for (auto const argument : call->arguments()) {
      if (!Resolve(argument,
                   type_factory()->NewArgument(call_value, position))) {
        DVLOG(0) << "argument[" << position
                 << "] should be subtype: " << *argument;
      }
      ++position;
    }
  }

  if (call_value->methods().empty()) {
    DVLOG(0) << "No matching methods for " << *call;
    Error(ErrorCode::TypeResolverMethodNoMatch, call);
    call_value->SetMethods({});
    ProduceResult(NewInvalidValue(call->callee()), call);
    return;
  }

  if (call_value->methods().size() == 1u) {
    ProduceUnifiedResult(
        NewLiteral(call_value->methods().front()->return_type()), call);
    return;
  }

  ProduceUnifiedResult(call_value, call);
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
  auto const literal_type = ValueOf(ast_type)->as<ir::Type>();
  if (!literal_type) {
    // Predefined type isn't defined.
    ProduceResult(NewInvalidValue(ast_literal), ast_literal);
    return;
  }
  auto const result = Unify(NewLiteral(literal_type), context_->value);
  auto const result_literal = result->as<ts::Literal>();
  if (!result_literal)
    return ProduceResult(NewInvalidValue(ast_literal), ast_literal);
  DCHECK(!ValueOf(ast_literal));
  semantics()->SetValue(
      ast_literal,
      ir_factory()->NewLiteral(result_literal->value(), ast_literal->token()));
  ProduceResult(result_literal, ast_literal);
}

void TypeResolver::VisitParameterReference(ast::ParameterReference* reference) {
  auto const value = variable_tracker_->RecordGet(reference->parameter());
  ProduceUnifiedResult(value, reference);
}

void TypeResolver::VisitVariableReference(ast::VariableReference* reference) {
  auto const value = variable_tracker_->RecordGet(reference->variable());
  ProduceUnifiedResult(value, reference);
}

}  // namespace compiler
}  // namespace elang
