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

class ConstraintFactory;

#define FOR_EACH_ABSTRACT_VALUE_CLASS(V) V(Value)

#define FOR_EACH_CONCRETE_VALUE_CLASS(V) \
  V(CallSite)                            \
  V(TypeVariable)                        \
  V(UnionType)

#define FOR_EACH_VALUE_CLASS(V)    \
  FOR_EACH_ABSTRACT_VALUE_CLASS(V) \
  FOR_EACH_CONCRETE_VALUE_CLASS(V)

class CallSite final : public Value {
 public:
  CallSite(ast::Call* call, const std::vector<Ir::Method*> methods);

  ZoneVector<ir::Method*> methods() const { return methods_; }

 private:
  ast::Call* const call_;
  ZoneVector<ir::Method*> methods_;

  DISALLOW_COPY_AND_ASSIGN(CallValue);
};

//////////////////////////////////////////////////////////////////////
//
// Constraint
//
#define FOR_EACH_ABSTRACT_CONSTRAINT_CLASS(V) V(Constraint)

#define FOR_EACH_CONCRETE_CONSTRAINT_CLASS(V) \
  V(ArgumentConstraint)                       \
  V(CallConstraint)                           \
  V(LiteralConstraint)                        \
  V(TypeConstraint)                           \
  V(TypeVariableConstraint)

#define FOR_EACH_CONSTRAINT_CLASS(V)    \
  FOR_EACH_ABSTRACT_CONSTRAINT_CLASS(V) \
  FOR_EACH_CONCRETE_CONSTRAINT_CLASS(V)

class Constraint : public ZoneAllocable {
 private:
  DISALLOW_COPY_AND_ASSIGN(Constraint);
};

class ArgumentConstraint final : public Constraint {
 public:
  ArgumentConstraint(CallConstraint* call, int position);

 private:
  CallConstraint* const call_;
  int const position_;

  DISALLOW_COPY_AND_ASSIGN(ArgumentConstraint);
};

class CallConstraint final : public Constraint {
 public:
  // Constructor creates |ArgumentConstraint| and stores into |arguments_|.
  CallConstraint(CallSite call, Constraint* output);

  const ZoneVector<Constraint*> arguments() const { return arguments_; }
  const Constraint* output() const { return output_; }

 private:
  CallSite const call_;
  Constraint* const output_;
  ZoneVector<Constraint*> arguments_;

  DISALLOW_COPY_AND_ASSIGN(CallConstraint);
};

class LiteralConstraint final : public Constraint {
 public:
  explicit LiteralConstraint(Constraint* constraint, Token* literal);

 private:
  Constraint* const constraint_;
  Token* const literal_;

  DISALLOW_COPY_AND_ASSIGN(LiteralConstraint);
};

class TypeConstraint : public Constraint {
 public:
  explicit TypeConstraint(ir::Type* type);

 private:
  ir::Type* const type_;

 private:
  DISALLOW_COPY_AND_ASSIGN(TypeConstraint);
};

class TypeVariableConstraint final : public Constraint {
 public:
  explicit TypeVariableConstraint(Token* name);

 private:
  Token* const name_;  // for debugging

 private:
  DISALLOW_COPY_AND_ASSIGN(TypeVariableConstraint);
};

//////////////////////////////////////////////////////////////////////
//
// ConstraintFactory
//
class ConstraintFactory final : public ZoneOwner {
 public:
  ConstraintFactory();
  ~ConstraintFactory() final = default;

  void Use(ast::Constraint* user, Constraint* using_value);

 private:
  SimpleDirectedGraph<Constraint*> dependency_graph_;
  ZoneUnorderedMap<ir::Type*, Constraint*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(ConstraintFactory);
};

ConstraintFactory::ConstraintFactory() {
}

void ConstraintFactory::Use(Constraint* user, Constraint* using_value) {
  DCHECK(!user->is<Literal>());
  DCHECK(!using_value->is<Literal>());
  dependency_graph_->AddEdge(user, using_value);
}

//////////////////////////////////////////////////////////////////////
//
// ConstraintFactoryUser
//
class ConstraintFactoryUser {
 public:
  explicit ConstraintFactoryUser(ConstraintFactory* factory);

  void AddConstraint(Constraint* user, Constraint* constraint);
  Constraint* GetOrNewConstraint(ir::Type* type);
  ArgumentConstraint* NewArgumentConstraint(Call* call, int position);
  Call* NewCall(ast::Call* call, const std::vector<ir::Method*> methods);
  InvalidConstraint* NewInvalidConstraint(ast::Node* node);
  Literal* NewLiteral(ir::Type* type);
  Null* NewNull(Constraint* value);
  void Use(Constraint* user, Constraint* using_value);

 private:
  ConstraintFactory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(ConstraintFactoryUser);
};

ConstraintFactoryUser::ConstraintFactoryUser(ConstraintFactory* factory)
    : factory_(factory) {
}

//////////////////////////////////////////////////////////////////////
//
// ConstraintBuilder
//
class ConstraintBuilder final : public Analyzer, public ConstraintFactoryUser {
 public:
  ConstraintBuilder(NameResolver* name_resolver,
                    ConstraintFactory* value_factory);
  ~ConstraintBuilder() final = default;

  Constraint* AddConstraint(ast::Expression* expression, Constraint* output);
  void DidAddAllConstraints();

 private:
  struct Context {
    bool has_result;
    Constraint* output;
    Constraint* result;
  };

  class ScopedContext {
   public:
    ScopedContext(ConstraintBuilder* builder,
                  ast::Expression* expression,
                  Constraint* output);
    ~ScopedContext();

    Constraint* GetResult() const;

   private:
    Context context;
    ConstraintBuilder* const builder_;
    Context* saved_context_;

    DISALLOW_COPY_AND_ASSIGN(ScopedContext);
  };

  // Returns list of ir::Method which takes at least |arity| arguments and
  // its value is subtype of |output|.
  std::vector<ir::Method*> CollectMethods(const ast::MethodGroup* method_group,
                                          Constraint* output,
                                          int arity);
  void ProcessMethodCall(ast::MethodGroup* method_group, ast::Call* ast_call);
  void ProduceResult(Constraint* result);

  // ast::Visitor
  void VisitCall(ast::Call* node);
  void VisitLiteral(ast::Literal* node);

  Context* context_;

  DISALLOW_COPY_AND_ASSIGN(ConstraintBuilder);
};

// ConstraintBuilder::ScopedContext
ConstraintBuilder::ScopedContext::ScopedContext(ConstraintBuilder* builder,
                                                ast::Expression* expression,
                                                Constraint* value)
    : builder_(builder), saved_context_(builder->context_) {
  context.has_result = false;
  context.output = output;
  context.result = nullptr;
  expression->Accept(builder);
}

ConstraintBuilder::ScopedContext::~ScopedContext() {
  DCHECK(context.has_result);
  builder_->context_ = saved_context_;
}

Constraint* ConstraintBuilder::ScopedContext::GetResult() {
  DCHECK(context_.has_result);
  return context.result;
}

// ConstraintBuilder
ConstraintBuilder::ConstraintBuilder(NameResolver* name_resolver,
                                     ConstraintFactory* value_factory)
    : Analyzer(name_resolver),
      ConstraintFactoryUser(value_factory),
      context_(nullptr) {
}

Constraint* ConstraintBuilder::AddConstraint(ast::Expression* expression,
                                             Constraint* output) {
  ScopedContext context(this, expression, output);
  return context.GetResult();
}

void ConstraintBuilder::DidAddAllConstraint() {
}

void ConstraintBuilder::ProcessMethodCall(ast::MethodGroup* method_group,
                                          ast::Call* ast_call) {
  auto const arity = static_cast<int>(ast_call.arguments().size());
  const auto methods = CollectMethods(method_group, context_.output, arity);
  auto const call = NewCall(ast_call, methods);
  std::vector<Constraint*> arguments;
  auto nth = 0;
  for (auto const ast_argument : ast_call->arguments()) {
    auto const argument = NewArgumentConstraint(call, nth);
    Use(call, argument);
    AddConstraint(ast_argument, argument);
    arguments.push_back(argument);
    ++nth;
  }
  call->SetArgumentConstraints(arguments);
}

void ConstraintBuilder::ProduceResult(Constraint* result) {
  DCHECK(!context_->has_result);
  AddConstraint(context->output, result);
  if (!result->is<Literal>())
    Use(context->output, result);
  context_->has_result = true;
  context_->result = result;
}

// ast::Visitor
void ConstraintBuilder::VisitCall(ast::Call* call) {
  auto const callee = ResolveReference(call->callee());
  if (auto const method_group = callee->as<ast::MethodGroup>()) {
    ProcessMethodCall(method_call, call);
    return;
  }
  Error(ErrorCode::TypeCalleeNotSupported);
}

void ConstraintBuilder::VisitLiteral(ast::Literal* literal) {
  if (auto const resolved = Resolve(node)) {
    ProduceResult(GetOrNewConstraint(resolve));
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
    ProduceResult(NewInvalidConstraint(literal));
    return;
  }
  auto const type = Resolve(ast_type);
  if (!type) {
    Error(ErrorCode::PredefinednameNameNotClass, type_name);
    Remember(node, nullptr);
    ProduceResult(NewInvalidConstraint(ast_type));
    return;
  }
  ProduceResult(GetOrNewConstraint(type));
}

//////////////////////////////////////////////////////////////////////
//
// ConstraintResolver
// Resolves constraints in built into |ConstraintFactory|, and stores following
// mapping into |NameResolver|:
//    * ast::Type -> ir:Type
//    * ast::Call -> ir:Method
class ConstraintResolver final : public ConstraintFactoryUser {
 public:
  explicit ConstraintResolver(ConstraintFactory* factory);
  ~ConstraintResolver() final = default;

  bool Run();

 private:
  Constraint* Evaluate(Constraint* constraint);
  void ResolveConstraint(Call* call);

  DISALLOW_COPY_AND_ASSIGN(ConstraintResolver);
};

ConstraintResolver::ConstraintResolver(ConstraintFactory* factory)
    : ConstraintFactoryUser(factory) {
}

void ConstraintResolver::ResolveConstraint(Call* call) {
  std::vector<Constraint*> arguments;
  for (auto const constraint : call->arguments()) {
    arguments.push_back(Evaluate(constraint));
  }
  std::vector<ir::Method*> methods;
  for (auto const method : call->methods()) {
    if (CanCall(method, output, arguments))
      methods.push_back(method);
  }
  if (methods.empty() {
    Error(ErrorCode::TypeCallNoMatc, call);
    return;
  }
  if (methods == call->methods())
    return;
}

void ConstraintResolver::Run() {
  for (auto const call : factory()->calls())
    ResolveConstraint(call);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TypeResolver::Impl
//
class TypeResolver::Impl final : public Analyzer {
 public:
  explicit Impl(NameResolver* name_resolver);
  ~Impl() final = default;

  ir::Method* ResolveCall(ast::Call* call);
  ir::Type* ResolveExpression(ast::Expression* expression);
  void AddExpression(ast::Expression* expression, ir::Type* result_type);
  bool Run();

 private:
  Constraint* Evaluate(ast::Expression* expression, Constraint* output_value);

  ConstraintBuilder builder_;
  ConstraintFactory factory_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

TypeResolver::Impl::Impl(NameResolver* name_resolver)
    : Analyzer(analyzer), builder_(name_resolver, &factory_) {
}

void TypeResolver::Impl::AddExpression(ast::Expression* expression,
                                       ir::Type* result_type) {
  if (auto const type_var = type->is<ir::TypeVariable>())
    Build(expression, NewVariable(type_var));
  else
    Build(expression, NewLiteral(result_type));
}

void TypeResolver::Impl::AddVariable(ast::Variable* variable) {
  if (!variable->expression())
    return;
  auto const type = ResolveTypeReference(variable->type());
  AddExpression(variable->expression(), type);
}

Constraint* TypeResolver::Build(ast::Expression* expression,
                                Constraint* output_value) {
  return builder_.Build(expression, output_value);
}

bool TypeResolver::Impl::Run() {
  builder_.DidAddAllConstraints();
  ConstraintResolver resolver(&factory_);
  return resolver.Run();
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
