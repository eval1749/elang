// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/class_analyzer.h"

#include "base/logging.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/compiler/analysis/analyzer.h"
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

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer::ConstantAnalyzer
//
class ClassAnalyzer::ConstantAnalyzer final : public Analyzer,
                                              public ast::Visitor {
 public:
  explicit ConstantAnalyzer(NameResolver* name_resolver);
  ~ConstantAnalyzer() = default;

  sm::Calculator& calculator() const { return *calculator_; }

  void AddDependency(ast::Node* from, ast::Node* to);
  sm::Semantic* Evaluate(sm::Semantic* owner, ast::Node* expression);
  void Postpone(ast::Node* node);

 private:
  SimpleDirectedGraph<ast::Node*> dependency_graph_;
  std::vector<ast::Node*> pending_nodes_;
  const std::unique_ptr<sm::Calculator> calculator_;

  DISALLOW_COPY_AND_ASSIGN(ConstantAnalyzer);
};

ClassAnalyzer::ConstantAnalyzer::ConstantAnalyzer(NameResolver* name_resolver)
    : Analyzer(name_resolver),
      calculator_(new sm::Calculator(name_resolver->session())) {
}

void ClassAnalyzer::ConstantAnalyzer::AddDependency(ast::Node* from,
                                                    ast::Node* to) {
  dependency_graph_.AddEdge(from, to);
}

void ClassAnalyzer::ConstantAnalyzer::Postpone(ast::Node* node) {
  pending_nodes_.push_back(node);
}

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer::Collector
//
class ClassAnalyzer::Collector final : public Analyzer, public ast::Visitor {
 public:
  explicit Collector(ConstantAnalyzer* analyzer);
  ~Collector() = default;

  void Run();

 private:
  sm::Calculator& calculator() const { return analyzer_->calculator(); }

  void AnalyzeEnumMember(sm::Enum* enum_type,
                         ast::EnumMember* ast_member,
                         ast::EnumMember* ast_previous_member);
  sm::Type* EnsureEnumBase(ast::Enum* enum_type);
  void FixEnumMember(sm::EnumMember* member, sm::Value* value);

  // ast::Visitor
  void VisitEnum(ast::Enum* node) final;
  void VisitField(ast::Field* node) final;
  void VisitMethod(ast::Method* node) final;

  ConstantAnalyzer* const analyzer_;

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

ClassAnalyzer::Collector::Collector(ConstantAnalyzer* analyzer)
    : Analyzer(analyzer->name_resolver()), analyzer_(analyzer) {
}

void ClassAnalyzer::Collector::AnalyzeEnumMember(
    sm::Enum* enum_type,
    ast::EnumMember* ast_member,
    ast::EnumMember* ast_previous_member) {
  auto const enum_base = enum_type->enum_base();
  auto const member_name = ast_member->name();
  calculator().SetContext(member_name);
  auto const member = SemanticOf(ast_member)->as<sm::EnumMember>();
  DCHECK(member) << ast_member;
  if (ast_member->expression()) {
    if (auto const literal = ast_member->expression()->as<ast::Literal>()) {
      editor()->FixEnumMember(member, calculator().NewIntValue(
                                          enum_base, literal->token()->data()));
      return;
    }
    analyzer_->Postpone(ast_member);
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
  analyzer_->AddDependency(ast_member, ast_previous_member);
}

sm::Type* ClassAnalyzer::Collector::EnsureEnumBase(ast::Enum* enum_type) {
  auto const type =
      enum_type->enum_base()
          ? analyzer_->ResolveTypeReference(enum_type->enum_base(), enum_type)
          : session()->PredefinedTypeOf(PredefinedName::Int32);
  if (calculator().IsIntType(type))
    return type;
  DCHECK(enum_type->enum_base()) << enum_type;
  analyzer_->Error(ErrorCode::SemanticEnumEnumBase, enum_type->enum_base());
  return session()->PredefinedTypeOf(PredefinedName::Int64);
}

void ClassAnalyzer::Collector::FixEnumMember(sm::EnumMember* member,
                                             sm::Value* value) {
  if (value->type() == member->owner()->enum_base())
    return editor()->FixEnumMember(member, value);
  analyzer_->Error(ErrorCode::SemanticEnumMemberValue, member->name(),
                   value->token());
}

// The entry point of |Collector|.
void ClassAnalyzer::Collector::Run() {
  Traverse(session()->global_namespace_body());
}

// ast::Visitor
void ClassAnalyzer::Collector::VisitEnum(ast::Enum* ast_enum) {
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

void ClassAnalyzer::Collector::VisitField(ast::Field* node) {
  DCHECK(node);
}

void ClassAnalyzer::Collector::VisitMethod(ast::Method* ast_method) {
  auto const clazz = SemanticOf(ast_method->owner())->as<sm::Class>();
  auto const method_name = ast_method->name();
  if (clazz->FindMember(method_name))
    return;
  factory()->NewMethodGroup(clazz, method_name);
}

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer::Resolver
//
class ClassAnalyzer::Resolver final : public ast::Visitor {
 public:
  explicit Resolver(ConstantAnalyzer* analyzer);
  ~Resolver() = default;

  sm::Editor* editor() const { return analyzer_->editor(); }
  sm::Factory* factory() const { return analyzer_->factory(); }
  CompilationSession* session() const { return analyzer_->session(); }

  void Run();

 private:
  sm::Semantic* SemanticOf(ast::Node* node) {
    return analyzer_->SemanticOf(node);
  }

  // ast::Visitor
  void VisitMethod(ast::Method* node) final;

  ConstantAnalyzer* const analyzer_;

  DISALLOW_COPY_AND_ASSIGN(Resolver);
};

ClassAnalyzer::Resolver::Resolver(ConstantAnalyzer* analyzer)
    : analyzer_(analyzer) {
}

// The entry point of |Resolver|.
void ClassAnalyzer::Resolver::Run() {
  Traverse(session()->global_namespace_body());
}

// ast::Visitor
void ClassAnalyzer::Resolver::VisitMethod(ast::Method* ast_method) {
  auto const clazz = SemanticOf(ast_method->owner())->as<sm::Class>();
  auto const method_name = ast_method->name();
  auto const method_group =
      clazz->FindMember(method_name)->as<sm::MethodGroup>();
  if (!method_group)
    return;
  auto const return_type = analyzer_->ResolveTypeReference(
      ast_method->return_type(), ast_method->owner());
  std::vector<sm::Parameter*> parameters(ast_method->parameters().size());
  parameters.resize(0);
  for (auto const parameter : ast_method->parameters()) {
    auto const parameter_type =
        analyzer_->ResolveTypeReference(parameter->type(), ast_method);
    parameters.push_back(
        factory()->NewParameter(parameter, parameter_type, nullptr));
  }

  auto const signature = factory()->NewSignature(return_type, parameters);
  auto const method = factory()->NewMethod(method_group, signature);
  analyzer_->SetSemanticOf(ast_method, method);

  // Check this size with existing signatures
  for (auto other : method_group->methods()) {
    if (method == other)
      continue;
    if (!other->signature()->IsIdenticalParameters(signature))
      continue;
    analyzer_->Error(other->return_type() == return_type
                         ? ErrorCode::ClassResolutionMethodDuplicate
                         : ErrorCode::ClassResolutionMethodConflict,
                     ast_method->name(), other->name());
  }
  // TODO(eval1749) Check whether |ast_method| overload methods in base class
  // with 'new', 'override' modifiers, or not
  // TODO(eval1749) Check |ast_method| not override static method.
}

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer
//
ClassAnalyzer::ClassAnalyzer(NameResolver* resolver)
    : analyzer_(new ConstantAnalyzer(resolver)) {
}

ClassAnalyzer::~ClassAnalyzer() {
}

void ClassAnalyzer::Run() {
  Collector(analyzer_.get()).Run();
  if (analyzer_->session()->HasError())
    return;
  Resolver(analyzer_.get()).Run();
}

}  // namespace compiler
}  // namespace elang
