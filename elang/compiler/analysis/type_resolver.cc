// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/type_resolver.h"

#include "base/logging.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/method_resolver.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/analysis/type_evaluator.h"
#include "elang/compiler/analysis/type_factory.h"
#include "elang/compiler/analysis/type_values.h"
#include "elang/compiler/analysis/variable_tracker.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
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
// TypeResolver::NumericType
//
struct TypeResolver::NumericType {
  enum class Kind { Float, Int, None, UInt };

  Kind kind;
  int size;

  NumericType(Kind kind, int size) : kind(kind), size(size) {}

  bool is_float() const { return kind == Kind::Float; }
  bool is_int() const { return kind == Kind::Int; }
  bool is_none() const { return kind == Kind::None; }
  bool is_uint() const { return kind == Kind::UInt; }
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

TypeResolver::NumericType TypeResolver::NumericTypeOf(ts::Value* value) const {
  if (value == float64_value())
    return NumericType(NumericType::Kind::Float, 64);
  if (value == float32_value())
    return NumericType(NumericType::Kind::Float, 32);
  if (value == int64_value())
    return NumericType(NumericType::Kind::Int, 64);
  if (value == int32_value())
    return NumericType(NumericType::Kind::Int, 32);
  if (value == int16_value())
    return NumericType(NumericType::Kind::Int, 16);
  if (value == int8_value())
    return NumericType(NumericType::Kind::Int, 8);
  if (value == uint64_value())
    return NumericType(NumericType::Kind::UInt, 64);
  if (value == uint32_value())
    return NumericType(NumericType::Kind::UInt, 32);
  if (value == uint16_value())
    return NumericType(NumericType::Kind::UInt, 16);
  if (value == uint8_value())
    return NumericType(NumericType::Kind::UInt, 8);
  return NumericType(NumericType::Kind::None, 0);
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
  if (result != empty_value())
    return;
  context_->result = NewInvalidValue(producer);
  if (context_->value == bool_value())
    Error(ErrorCode::TypeResolverExpressionNotBool, producer);
  else
    Error(ErrorCode::TypeResolverExpressionInvalid, producer);
}

// Set unified value as result.
void TypeResolver::ProduceUnifiedResult(ts::Value* result,
                                        ast::Node* producer) {
  ProduceResult(Unify(result, context_->value), producer);
}

ts::Value* TypeResolver::PromoteNumericType(NumericType left_type,
                                            NumericType right_type) const {
  if (left_type.is_none())
    return PromoteNumericType(right_type);

  if (right_type.is_none())
    return PromoteNumericType(left_type);

  // Promote to Float
  if (left_type.is_float() && right_type.is_float()) {
    return left_type.size == 64 || right_type.size == 64 ? float64_value()
                                                         : float32_value();
  }

  if (left_type.is_float())
    return left_type.size == 64 ? float64_value() : float32_value();

  if (right_type.is_float())
    return right_type.size == 64 ? float64_value() : float32_value();

  // Promote to 64-bit or 32-bit integer.
  if (left_type.kind != right_type.kind)
    return empty_value();

  if (left_type.is_uint())
    return left_type.size == 64 || right_type.size == 64 ? uint64_value()
                                                         : uint32_value();
  return left_type.size == 64 || right_type.size == 64 ? int64_value()
                                                       : int32_value();
}

ts::Value* TypeResolver::PromoteNumericType(NumericType type) const {
  switch (type.kind) {
    case NumericType::Kind::Float:
      return type.size == 64 ? float64_value() : float32_value();
    case NumericType::Kind::Int:
      return type.size == 64 ? int64_value() : int32_value();
    case NumericType::Kind::UInt:
      return type.size == 64 ? uint64_value() : uint32_value();
  }
  return empty_value();
}

void TypeResolver::ProduceSemantics(ts::Value* value, ast::Node* ast_node) {
  if (auto const literal = value->as<ts::Literal>())
    SetSemanticOf(ast_node, literal->value());
  ProduceUnifiedResult(value, ast_node);
}

// The entry point of |TypeResolver|. When |upper_bound| is |EmptyValue|, we
// assume |expression| is analyzed in error context.
ts::Value* TypeResolver::Resolve(ast::Expression* expression,
                                 ts::Value* upper_bound) {
  auto const value = upper_bound == empty_value() ? any_value() : upper_bound;
  ScopedContext context(this, value, expression);
  Traverse(expression);
  auto const result = context_->result;
  if (!result || result == empty_value())
    return NewInvalidValue(expression);
  return result;
}

ts::Value* TypeResolver::ResolveAsBool(ast::Expression* expression) {
  auto const result = Resolve(expression, bool_value());
  if (result != bool_value()) {
    // TODO(eval1749) Looking for |implicit operator bool()| and
    // |static bool operator true(Ty)|
    return empty_value();
  }
  return result;
}

sm::Semantic* TypeResolver::ResolveReference(ast::Expression* expression) {
  return name_resolver()->ResolveReference(expression, context_method_);
}

ts::Value* TypeResolver::Unify(ts::Value* value1, ts::Value* value2) {
  ts::Evaluator evaluator(type_factory());
  auto const result = evaluator.Unify(value1, value2);
  if (result == empty_value()) {
    DVLOG(0) << "Unify(" << *value1 << ", " << *value2 << ") yields empty.";
  }
  return result;
}

sm::Semantic* TypeResolver::SemanticOf(ast::Node* node) {
  return analysis()->SemanticOf(node);
}

// ast::Visitor
void TypeResolver::DoDefaultVisit(ast::Node* node) {
  Error(ErrorCode::TypeResolverExpressionNotYetImplemented, node);
}

// Check |array| is array type and |index|+ are integer type.
void TypeResolver::VisitArrayAccess(ast::ArrayAccess* node) {
  auto const array = Resolve(node->array(), any_value());
  auto const array_type =
      array->as<ts::Literal>()->value()->as<sm::ArrayType>();
  if (!array_type) {
    Error(ErrorCode::TypeResolverArrayAccessArray, node->array());
    return;
  }
  if (array_type->rank() != static_cast<int>(node->indexes().size()))
    Error(ErrorCode::TypeResolverArrayAccessRank, node);
  for (auto index : node->indexes()) {
    ts::Evaluator evaluator(type_factory());
    // TODO(eval1749) We should try to unify type of |index| with integral
    // type rather than evaluate type expression.
    auto const index_type = evaluator.Evaluate(Resolve(index, any_value()));
    auto const result = NumericTypeOf(index_type);
    if (result.is_int() || result.is_uint()) {
      continue;
    }
    Error(ErrorCode::TypeResolverArrayAccessIndex, index);
  }
  ProduceResult(type_factory()->NewLiteral(array_type->element_type()), node);
}

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
  if (auto const reference = lhs->as<ast::ArrayAccess>()) {
    auto const element_value = Resolve(reference, any_value());
    ProduceResolved(rhs, element_value, assignment);
    return;
  }
  if (auto const reference = lhs->as<ast::NameReference>()) {
    auto const value = ResolveReference(reference);
    DCHECK(value) << "NYI Assign to field " << *lhs;
    return;
  }
  if (auto const reference = lhs->as<ast::MemberAccess>()) {
    auto const value = ResolveReference(reference);
    DCHECK(value) << "NYI Assign to field " << *lhs;
    return;
  }
  Error(ErrorCode::TypeResolverAssignmentLeftValue, lhs);
}

void TypeResolver::VisitBinaryOperation(ast::BinaryOperation* ast_node) {
  // TODO(eval1749) Support type variables in binary operation
  // TODO(eval1749) Support user defined binary operator

  if (ast_node->op() == TokenType::NullOr) {
    // T operator??(T?, T)
    // T operator??(T, T) T is reference type
    // TODO(eval1749) left should be nullable.
    auto const left = Resolve(ast_node->left(), any_value());
    auto const right = Resolve(ast_node->right(), any_value());
    if (left == empty_value() || right == empty_value())
      return;
    ProduceSemantics(right, ast_node);
    return;
  }

  if (ast_node->is_conditional()) {
    // bool operator&&(bool, bool)
    // bool operator||(bool, bool)
    ResolveAsBool(ast_node->left());
    ResolveAsBool(ast_node->right());
    ProduceUnifiedResult(bool_value(), ast_node);
    return;
  }

  // TODO(eval1749) We should try to unify type of |index| with numeric
  // type rather than evaluate type expression.
  ts::Evaluator evaluator(type_factory());
  auto const left = evaluator.Evaluate(Resolve(ast_node->left(), any_value()));
  auto const right =
      evaluator.Evaluate(Resolve(ast_node->right(), any_value()));
  if (ast_node->is_equality()) {
    // bool operator==(T, T)
    // bool operator!=(T, T)
    // TODO(eval1749) Make left and right to same type.
    if (left != right)
      Error(ErrorCode::TypeResolverBinaryOperationEquality, ast_node);
    ProduceUnifiedResult(bool_value(), ast_node);
    return;
  }

  auto const left_type = NumericTypeOf(left);
  auto const right_type = NumericTypeOf(right);

  if (left_type.is_none() && right_type.is_none()) {
    Error(ErrorCode::TypeResolverBinaryOperationNumeric, ast_node->left());
    Error(ErrorCode::TypeResolverBinaryOperationNumeric, ast_node->right());
    return;
  }

  if (ast_node->is_bitwise_shift()) {
    // int32 operator<<(int32, int32)
    // int64 operator<<(int64, int64)
    // uint32 operator<<(uint32, uint32)
    // uint64 operator<<(uint64, uint64)
    if (!right_type.is_int() && right_type.size != 32) {
      Error(ErrorCode::TypeResolverBinaryOperationShift, ast_node->right());
      return;
    }
    if (left_type.is_int()) {
      auto const result = left_type.size == 64 ? int64_value() : int32_value();
      ProduceSemantics(result, ast_node);
      return;
    }
    if (left_type.is_uint()) {
      auto const result =
          left_type.size == 64 ? uint64_value() : uint32_value();
      ProduceSemantics(result, ast_node);
      return;
    }
    Error(ErrorCode::TypeResolverBinaryOperationNumeric, ast_node->left());
    return;
  }

  // On arithmetic and bitwise operation, both operands should be promoted
  // to same numeric type.
  auto const result = PromoteNumericType(left_type, right_type);
  if (ast_node->is_arithmetic()) {
    ProduceSemantics(result, ast_node);
    return;
  }

  if (ast_node->is_bitwise()) {
    auto const result_type = NumericTypeOf(result);
    if (result_type.is_int() || result_type.is_uint()) {
      ProduceSemantics(result, ast_node);
      return;
    }
    if (left_type.is_float())
      Error(ErrorCode::TypeResolverBinaryOperationNumeric, ast_node->left());
    if (right_type.is_float())
      Error(ErrorCode::TypeResolverBinaryOperationNumeric, ast_node->right());
    return;
  }

  if (ast_node->is_relational()) {
    ProduceUnifiedResult(bool_value(), ast_node);
    SetSemanticOf(ast_node, result->as<ts::Literal>()->value());
    return;
  }

  NOTREACHED() << "Unknown binary operation: " << *ast_node;
}

// Bind applicable methods to |call->callee|.
void TypeResolver::VisitCall(ast::Call* call) {
  auto const callee = ResolveReference(call->callee());
  if (!callee)
    return;
  auto const method_group = callee->as<sm::MethodGroup>();
  if (!method_group) {
    // TODO(eval1749) NYI call site other than method call.
    Error(ErrorCode::TypeResolverCalleeNotSupported, call->callee());
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
        return;
      }
      if (!parameter->is_rest())
        ++parameters;
    }
    ProduceUnifiedResult(NewLiteral(method->return_type()), call);
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
    return;
  }

  if (call_value->methods().size() == 1u) {
    ProduceUnifiedResult(
        NewLiteral(call_value->methods().front()->return_type()), call);
    return;
  }

  ProduceUnifiedResult(call_value, call);
}

void TypeResolver::VisitConditional(ast::Conditional* ast_node) {
  ResolveAsBool(ast_node->condition());
  auto const true_value = Resolve(ast_node->true_expression(), any_value());
  auto const false_value = Resolve(ast_node->false_expression(), any_value());
  // TODO(eval1749) Type of conditional expression is
  //   |true_value| if implicit_cast(true_value) -> false_value and
  //                 no implicit_cast(false_value) -> true_value
  //   |false_value| if implicit_cast(false_value) -> true_value and
  //                 no implicit_cast(true_value) -> false_value
  if (true_value != false_value) {
    Error(ErrorCode::TypeResolverConditionalNotMatch,
          ast_node->true_expression(), ast_node->false_expression());
    return;
  }
  ProduceUnifiedResult(Unify(false_value, true_value), ast_node);
}

// Post/pre decrement/increment
//
void TypeResolver::VisitIncrementExpression(ast::IncrementExpression* node) {
  auto const place = node->expression();
  ts::Evaluator evaluator(type_factory());
  // TODO(eval1749) We should try to unify type of |index| with numeric
  // type rather than evaluate type expression.
  auto const operand = evaluator.Evaluate(Resolve(place, any_value()));
  auto const numeric_type = NumericTypeOf(operand);
  if (numeric_type.is_none()) {
    Error(ErrorCode::TypeResolverIncrementExpressionType, node->expression());
    return;
  }
  if (!place->is<ast::VariableReference>()) {
    // TODO(eval1749) NYI: checking field access and property access
    Error(ErrorCode::TypeResolverIncrementExpressionPlace, node->expression());
    return;
  }
  ProduceSemantics(PromoteNumericType(numeric_type), node);
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
  auto const literal_type = session()->PredefinedTypeOf(token->literal_type());
  if (!literal_type) {
    // Predefined type isn't defined.
    return;
  }
  auto const result = Unify(NewLiteral(literal_type), context_->value);
  auto const result_literal = result->as<ts::Literal>();
  if (!result_literal)
    return;
  DCHECK(!SemanticOf(ast_literal));
  SetSemanticOf(ast_literal,
                semantic_factory()->NewLiteral(result_literal->value(),
                                               ast_literal->token()));
  ProduceResult(result_literal, ast_literal);
}

void TypeResolver::VisitNameReference(ast::NameReference* node) {
  auto const semantic = ResolveReference(node);
  if (!semantic)
    return;
  SetSemanticOf(node, semantic);
  if (auto const field = semantic->as<sm::Field>())
    return ProduceUnifiedResult(NewLiteral(field->type()), node);
}

void TypeResolver::VisitParameterReference(ast::ParameterReference* reference) {
  auto const value = variable_tracker_->RecordGet(reference->parameter());
  ProduceUnifiedResult(value, reference);
}

//  '!' bool
//  '~' int|uint
//  '+' numeric
//  '-' numeric
void TypeResolver::VisitUnaryOperation(ast::UnaryOperation* node) {
  if (node->op() == TokenType::Not)
    return ProduceUnifiedResult(ResolveAsBool(node->expression()), node);

  auto const operand = Resolve(node->expression(), any_value());
  auto const numeric_type = NumericTypeOf(operand);
  if (numeric_type.is_none()) {
    Error(ErrorCode::TypeResolverUnaryOperationType, node->expression());
    return;
  }
  if (node->op() == TokenType::BitNot && numeric_type.is_float()) {
    Error(ErrorCode::TypeResolverUnaryOperationType, node->expression());
    return;
  }
  ProduceSemantics(PromoteNumericType(numeric_type), node);
}

void TypeResolver::VisitVariableReference(ast::VariableReference* reference) {
  auto const value = variable_tracker_->RecordGet(reference->variable());
  ProduceUnifiedResult(value, reference);
}

}  // namespace compiler
}  // namespace elang
