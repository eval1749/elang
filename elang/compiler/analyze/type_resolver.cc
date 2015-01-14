// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/type_resolver.h"

#include "base/logging.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"

namespace elang {
namespace compiler {

namespace {
class Value : public ZoneAllocable {
 private:
  DISALLOW_COPY_AND_ASSIGN(Value);
};

class Argument : public Value {
 public:
  Argument(Call* call, int position);

 private:
  Call* const call_;
  int const position_;

  DISALLOW_COPY_AND_ASSIGN(Argument);
};

class Call : public Value {
 public:
  Call(ast::MethodGroup* method_group,
       Value* value_type,
       const std::vector<Value*>& arguments);

 private:
  ast::MethodGroup* const method_group_;
  Value* const value_type_;
  ZoneVector<Value*> arguments_;

  DISALLOW_COPY_AND_ASSIGN(Call);
};

class Literal : public Value {
 public:
  explicit Literal(ir::Type* type);

 private:
  ir::Type* const type_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

class Null : public Value {
 public:
  explicit Null(Value* node);

 private:
  Value* const node_;

  DISALLOW_COPY_AND_ASSIGN(Null);
};

class Variable : public Value {
 public:
  explicit Variable(Token* name);

 private:
  Token* const name_;  // for debugging

 private:
  DISALLOW_COPY_AND_ASSIGN(Variable);
};

//////////////////////////////////////////////////////////////////////
//
// TypeEvaluator
//
class TypeEvaluator final : public Analyzer, public ValueFactoryUser {
 public:
  TypeEvaluator(NameResolver* name_resolver, ValueFactory* value_factory);
  ~TypeEvaluator() final = default;

  Value* Evaluate(ast::Expression* expression, Value* output);
  bool Finish();

 private:
  struct Context {
    bool has_result;
    Value* output;
    Value* result;
  };

  class ScopedContext {
   public:
    ScopedContext(TypeEvaluator* evaluator,
                  ast::Expression* expression,
                  Value* output);
    ~ScopedContext();

    Value* GetResult() const;

   private:
    Context context;
    TypeEvaluator* const evaluator_;
    Context* saved_context_;

    DISALLOW_COPY_AND_ASSIGN(ScopedContext);
  };

  void ProcessMethodCall(ast::MethodGroup* method_group, ast::Call* ast_call);
  void ProduceResult(Value* result);
  void TryFix(Value* value);
  void Use(ast::Value* user, Value* using_value);

  // ast::Visitor
  void VisitCall(ast::Call* node);
  void VisitLiteral(ast::Literal* node);

  Context* context_;
  SimpleDirectedGraph<Value*> dependency_graph_;

  DISALLOW_COPY_AND_ASSIGN(TypeEvaluator);
};

// TypeEvaluator::ScopedContext
TypeEvaluator::ScopedContext::ScopedContext(TypeEvaluator* evaluator,
                                            ast::Expression* expression,
                                            Value* value)
    : evaluator_(evaluator), saved_context_(evaluator->context_) {
  context.has_result = false;
  context.output = output;
  context.result = nullptr;
  expression->Accept(evaluator);
}

TypeEvaluator::ScopedContext::~ScopedContext() {
  DCHECK(context.has_result);
  evaluator_->context_ = saved_context_;
}

Value* TypeEvaluator::ScopedContext::GetResult() {
  DCHECK(context_.has_result);
  return context.result;
}

// TypeEvaluator
TypeEvaluator::TypeEvaluator(NameResolver* name_resolver,
                             ValueFactory* value_factory)
    : Analyzer(name_resolver),
      ValueFactoryUser(value_factory),
      context_(nullptr) {
}

Value* TypeEvaluator::Evaluate(ast::Expression* expression, Value* output) {
  ScopedContext context(this, expression, output);
  return context.GetResult();
}

bool TypeEvaluator::Finish() {
  for (auto const value : dependency_graph_.GetAllVertices())
    TryFix(value);
  // Check everything resolved.
  for (auto const user : dependency_graph_.GetAllVertices()) {
    if (user->HasInEdge())
      return false;
  }
  return true;
}

void TypeEvaluator::ProcessMethodCall(ast::MethodGroup* method_group,
                                      ast::Call* ast_call) {
  auto methods = CollectMethods(method_group,
                                context_.output,
                                static_cast<int>(ast_call.arguments().size()));
  auto const call = NewCall(ast_call, methods);
  std::vector<Value*> arguments;
  for (auto const ast_argument : ast_call->arguments()) {
    auto const argument = NewArgument(call, static_cast<int>(arguments.size()));
    Use(argument, call);
    Evaluate(ast_argument, argument);
  }
  ProduceValue(call);
}

void TypeEvaluator::ProduceResult(Value* result) {
  DCHECK(!context_->has_result);
  context->output->AddConstraint(result);
  if (!result->is<Literal>())
    Use(context->output, result);
  context_->has_result = true;
  context_->result = result;
}

void TypeEvaluator::TryFix(Value* value) {
  if (value->HasOutEdge(value))
    return;
  auto const users = dependency_graph_->GetInEdges(value);
  for (auto const user : users)
    TryFix(user);
}

void TypeEvaluator::Use(Value* user, Value* using_value) {
  DCHECK(!user->is<Literal>());
  DCHECK(!using_value->is<Literal>());
  dependency_graph_->AddEdge(user, using_value);
}

// ast::Visitor
void TypeEvaluator::VisitCall(ast::Call* call) {
  auto const callee = ResolveReference(call->callee());
  if (auto const method_group = callee->as<ast::MethodGroup>()) {
    ProcessMethodCall(method_call, call);
    return;
  }
  Error(ErrorCode::TypeCalleeNotSupported);
}

void TypeEvaluator::VisitLiteral(ast::Literal* literal) {
  if (auto const resolved = Resolve(node)) {
    ProduceResult(GetOrNewValue(resolve));
    return;
  }
  if (node->token() == TokenType::NullLiteral) {
    ProduceResult(NewNull(output_type_));
    return;
  }
  auto const literal_type = node->token()->literal_type();
  auto const type_name = session()->name_for(literal_type);
  auto const ast_type = session()->system_namespace()->FindMember(type_name);
  if (!ast_type) {
    Error(ErrorCode::PredefinednameNameNotFound, type_name);
    Remember(node, nullptr);
    ProduceResult(NewInvalidValue(literal));
    return;
  }
  auto const type = Resolve(ast_type);
  if (!type) {
    Error(ErrorCode::PredefinednameNameNotClass, type_name);
    Remember(node, nullptr);
    ProduceResult(NewInvalidValue(ast_type));
    return;
  }
  ProduceResult(GetOrNewValue(type));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TypeResolver::Impl
//
class TypeResolver::Impl final : public Analyzer, public ValueFactoryOwner {
 public:
  explicit Impl(NameResolver* name_resolver);
  ~Impl() final = default;

  ir::Method* ResolveCall(ast::Call* call);
  ir::Type* ResolveExpression(ast::Expression* expression);
  void AddExpression(ast::Expression* expression, ir::Type* result_type);
  bool Run();

 private:
  Value* Evaluate(ast::Expression* expression, Value* output_value);

  TypeEvaluator evaluator_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

TypeResolver::Impl::Impl(NameResolver* name_resolver)
    : Analyzer(analyzer), evaluator_(name_resolver, value_factory()) {
}

void TypeResolver::Impl::AddExpression(ast::Expression* expression,
                                       ir::Type* result_type) {
  if (auto const type_var = type->is<ir::TypeVariable>())
    Evaluate(expression, NewVariable(type_var);
  else
    Evaluate(expression, NewLiteral(result_type));
}

void TypeResolver::Impl::AddVariable(ast::Variable* variable) {
  if (!variable->expression())
    return;
  auto const type = ResolveTypeReference(variable->type());
  AddExpression(variable->expression(), type);
}

Value* TypeResolver::Evaluate(ast::Expression* expression,
                              Value* output_value) {
  return evaluator_.Evaluate(expression, output_value);
}

bool TypeResolver::Run() {
  return evaluator_.Finish();
}

//////////////////////////////////////////////////////////////////////
//
// TypeResolver
//
TypeResolver::TypeResolver(NameResolver* name_resolver)
    : impl_(new impl(name_resolver)) {
}

TypeResolver::~TypeResolver() {
}

void TypeResolver::AddExpression(ast::Expression* expression,
                                 ir::Type* result_type) {
  impl_->AddExpression(expression, result_type);
}

void TypeResolver::AddVariable(ast::Variable* variable) {
  impl_->AddVariable(variable);
}

bool TypeResolver::Run() {
  return impl_->Run();
}

}  // namespace compiler
}  // namespace elang
