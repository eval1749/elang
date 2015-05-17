// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/class_analyzer.h"

#include "base/logging.h"
#include "elang/compiler/analysis/analyzer.h"
#include "elang/compiler/analysis/const_expr_evaluator.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/calculator.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {

namespace {

//////////////////////////////////////////////////////////////////////
//
// Collector
//
class Collector final : public Analyzer, public ast::Visitor {
 public:
  explicit Collector(ConstExprEvaluator* evaluator);
  ~Collector() = default;

  void Run();

 private:
  sm::Calculator& calculator() const { return evaluator_->calculator(); }

  void AnalyzeEnumMember(sm::Enum* enum_type,
                         ast::EnumMember* ast_member,
                         ast::EnumMember* ast_previous_member);
  sm::Type* EnsureEnumBase(ast::Enum* enum_type);
  void FixEnumMember(sm::EnumMember* member, sm::Value* value);

  // ast::Visitor
  void VisitEnum(ast::Enum* node) final;
  void VisitField(ast::Field* node) final;
  void VisitMethod(ast::Method* node) final;

  ConstExprEvaluator* const evaluator_;

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

Collector::Collector(ConstExprEvaluator* analyzer)
    : Analyzer(analyzer->name_resolver()), evaluator_(analyzer) {
}

void Collector::AnalyzeEnumMember(sm::Enum* enum_type,
                                  ast::EnumMember* ast_member,
                                  ast::EnumMember* ast_previous_member) {
  auto const enum_base = enum_type->enum_base();
  auto const member_name = ast_member->name();
  calculator().SetContext(member_name);
  auto const member = SemanticOf(ast_member)->as<sm::EnumMember>();
  DCHECK(member) << ast_member;
  if (ast_member->expression()) {
    auto const value = evaluator_->Evaluate(member, ast_member->expression());
    if (!calculator().IsTypeOf(value, enum_base)) {
      Error(ErrorCode::AnalyzeExpressionType, ast_member->expression(),
            enum_base->name());
      return;
    }
    editor()->FixEnumMember(member, calculator().CastAs(value, enum_base));
    return;
  }
  if (!ast_previous_member) {
    editor()->FixEnumMember(member, calculator().Zero(enum_base));
    return;
  }
  auto const previous_member =
      SemanticOf(ast_previous_member)->as<sm::EnumMember>();
  if (previous_member->has_value()) {
    editor()->FixEnumMember(member,
                            calculator().Add(previous_member->value(), 1));
    return;
  }
  evaluator_->AddDependency(member, previous_member);
}

sm::Type* Collector::EnsureEnumBase(ast::Enum* enum_type) {
  auto const type =
      enum_type->enum_base()
          ? evaluator_->ResolveTypeReference(enum_type->enum_base(), enum_type)
          : session()->PredefinedTypeOf(PredefinedName::Int32);
  if (calculator().IsIntType(type))
    return type;
  DCHECK(enum_type->enum_base()) << enum_type;
  evaluator_->Error(ErrorCode::SemanticEnumEnumBase, enum_type->enum_base());
  return session()->PredefinedTypeOf(PredefinedName::Int64);
}

void Collector::FixEnumMember(sm::EnumMember* member, sm::Value* value) {
  if (value->type() == member->owner()->enum_base())
    return editor()->FixEnumMember(member, value);
  evaluator_->Error(ErrorCode::SemanticEnumMemberValue, member->name(),
                    value->token());
}

// The entry point of |Collector|.
void Collector::Run() {
  Traverse(session()->global_namespace_body());
}

// ast::Visitor
void Collector::VisitEnum(ast::Enum* ast_enum) {
  auto const enum_base = EnsureEnumBase(ast_enum);
  auto const enum_type = SemanticOf(ast_enum)->as<sm::Enum>();
  editor()->FixEnumBase(enum_type, enum_base);

  ast::EnumMember* ast_previous = nullptr;
  for (auto const ast_node : ast_enum->members()) {
    auto const ast_member = ast_node->as<ast::EnumMember>();
    AnalyzeEnumMember(enum_type, ast_member, ast_previous);
    ast_previous = ast_member;
  }
}

void Collector::VisitField(ast::Field* node) {
  DCHECK(node);
}

void Collector::VisitMethod(ast::Method* ast_method) {
  auto const clazz = SemanticOf(ast_method->owner())->as<sm::Class>();
  auto const method_name = ast_method->name();
  if (clazz->FindMember(method_name))
    return;
  factory()->NewMethodGroup(clazz, method_name);
}

//////////////////////////////////////////////////////////////////////
//
// Resolver
//
class Resolver final : public ast::Visitor {
 public:
  explicit Resolver(ConstExprEvaluator* analyzer);
  ~Resolver() = default;

  sm::Editor* editor() const { return evaluator_->editor(); }
  sm::Factory* factory() const { return evaluator_->factory(); }
  CompilationSession* session() const { return evaluator_->session(); }

  void Run();

 private:
  sm::Semantic* SemanticOf(ast::Node* node) {
    return evaluator_->SemanticOf(node);
  }

  // ast::Visitor
  void VisitMethod(ast::Method* node) final;

  ConstExprEvaluator* const evaluator_;

  DISALLOW_COPY_AND_ASSIGN(Resolver);
};

Resolver::Resolver(ConstExprEvaluator* analyzer) : evaluator_(analyzer) {
}

// The entry point of |Resolver|.
void Resolver::Run() {
  Traverse(session()->global_namespace_body());
}

// ast::Visitor
void Resolver::VisitMethod(ast::Method* ast_method) {
  auto const clazz = SemanticOf(ast_method->owner())->as<sm::Class>();
  auto const method_name = ast_method->name();
  auto const method_group =
      clazz->FindMember(method_name)->as<sm::MethodGroup>();
  if (!method_group)
    return;
  auto const return_type = evaluator_->ResolveTypeReference(
      ast_method->return_type(), ast_method->owner());
  std::vector<sm::Parameter*> parameters(ast_method->parameters().size());
  parameters.resize(0);
  for (auto const parameter : ast_method->parameters()) {
    auto const parameter_type =
        evaluator_->ResolveTypeReference(parameter->type(), ast_method);
    parameters.push_back(
        factory()->NewParameter(parameter, parameter_type, nullptr));
  }

  auto const signature = factory()->NewSignature(return_type, parameters);
  auto const method = factory()->NewMethod(method_group, signature);
  evaluator_->SetSemanticOf(ast_method, method);

  // Check this size with existing signatures
  for (auto other : method_group->methods()) {
    if (method == other)
      continue;
    if (!other->signature()->IsIdenticalParameters(signature))
      continue;
    evaluator_->Error(other->return_type() == return_type
                          ? ErrorCode::ClassResolutionMethodDuplicate
                          : ErrorCode::ClassResolutionMethodConflict,
                      ast_method->name(), other->name());
  }
  // TODO(eval1749) Check whether |ast_method| overload methods in base class
  // with 'new', 'override' modifiers, or not
  // TODO(eval1749) Check |ast_method| not override static method.
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer
//
ClassAnalyzer::ClassAnalyzer(NameResolver* resolver)
    : evaluator_(new ConstExprEvaluator(resolver)) {
}

ClassAnalyzer::~ClassAnalyzer() {
}

void ClassAnalyzer::Run() {
  Collector(evaluator_.get()).Run();
  if (evaluator_->session()->HasError())
    return;
  Resolver(evaluator_.get()).Run();
}

}  // namespace compiler
}  // namespace elang
