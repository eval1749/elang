// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/class_analyzer.h"

#include "base/logging.h"
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
// ClassAnalyzer::Collector
//
class ClassAnalyzer::Collector final : public ast::Visitor {
 public:
  explicit Collector(ClassAnalyzer* analyzer);
  ~Collector() = default;

  sm::Editor* editor() const { return analyzer_->editor(); }
  sm::Factory* factory() const { return analyzer_->factory(); }
  CompilationSession* session() const { return analyzer_->session(); }

  void Run();

 private:
  sm::EnumMember* AnalyzeEnumMember(sm::Enum* enum_type,
                                    ast::EnumMember* ast_member,
                                    ast::EnumMember* ast_previous_member);
  sm::Type* EnsureEnumBase(ast::Enum* enum_type);
  void FixEnumMember(sm::EnumMember* member, sm::Value* value);

  sm::Semantic* SemanticOf(ast::Node* node) {
    return analyzer_->SemanticOf(node);
  }

  // ast::Visitor
  void VisitEnum(ast::Enum* node) final;
  void VisitField(ast::Field* node) final;
  void VisitMethod(ast::Method* node) final;

  ClassAnalyzer* const analyzer_;
  std::unique_ptr<sm::Calculator> calculator_;

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

ClassAnalyzer::Collector::Collector(ClassAnalyzer* analyzer)
    : analyzer_(analyzer),
      calculator_(new sm::Calculator(analyzer->session())) {
}

sm::EnumMember* ClassAnalyzer::Collector::AnalyzeEnumMember(
    sm::Enum* enum_type,
    ast::EnumMember* ast_member,
    ast::EnumMember* ast_previous_member) {
  auto const enum_base = enum_type->enum_base();
  auto const member_name = ast_member->name();
  calculator_->SetContext(member_name);
  if (ast_member->expression()) {
    if (auto const literal = ast_member->expression()->as<ast::Literal>()) {
      return factory()->NewEnumMember(
          enum_type, member_name,
          calculator_->NewIntValue(enum_base, literal->token()->data()));
    }
    analyzer_->Postpone(ast_member);
    return factory()->NewEnumMember(enum_type, member_name, nullptr);
  }
  if (!ast_previous_member) {
    return factory()->NewEnumMember(enum_type, member_name,
                                    calculator_->Zero(enum_base));
  }
  auto const previous_member =
      SemanticOf(ast_previous_member)->as<sm::EnumMember>();
  if (previous_member->is_bound()) {
    return factory()->NewEnumMember(
        enum_type, member_name, calculator_->Add(previous_member->value(), 1));
  }
  analyzer_->AddDependency(ast_member, ast_previous_member);
  return factory()->NewEnumMember(enum_type, member_name, nullptr);
}

sm::Type* ClassAnalyzer::Collector::EnsureEnumBase(ast::Enum* enum_type) {
  auto const type =
      enum_type->enum_base()
          ? analyzer_->ResolveTypeReference(enum_type->enum_base(), enum_type)
          : session()->PredefinedTypeOf(PredefinedName::Int32);
  if (!calculator_->IsIntType(type)) {
    DCHECK(enum_type->enum_base()) << enum_type;
    analyzer_->Error(ErrorCode::SemanticEnumEnumBase, enum_type->enum_base());
    return session()->PredefinedTypeOf(PredefinedName::Int64);
  }
  return type;
}

void ClassAnalyzer::Collector::FixEnumMember(sm::EnumMember* member,
                                             sm::Value* value) {
  if (value->type() != member->owner()->enum_base()) {
    analyzer_->Error(ErrorCode::SemanticEnumMemberValue, member->name(),
                     value->token());
    return;
  }
  analyzer_->editor()->FixEnumMember(member, value);
}

// The entry point of |ClassAnalyzer|.
void ClassAnalyzer::Collector::Run() {
  VisitNamespaceBody(session()->global_namespace_body());
}

// ast::Visitor
void ClassAnalyzer::Collector::VisitEnum(ast::Enum* ast_enum) {
  auto const enum_base = EnsureEnumBase(ast_enum);
  auto const outer = SemanticOf(ast_enum->parent());
  auto const enum_type = factory()->NewEnum(outer, ast_enum->name(), enum_base);
  analyzer_->SetSemanticOf(ast_enum, enum_type);

  ast::EnumMember* ast_previous = nullptr;
  for (auto const ast_node : ast_enum->members()) {
    auto const ast_member = ast_node->as<ast::EnumMember>();
    auto const member = AnalyzeEnumMember(enum_type, ast_member, ast_previous);
    analyzer_->SetSemanticOf(ast_member, member);
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
  explicit Resolver(ClassAnalyzer* analyzer);
  ~Resolver() = default;

  sm::Editor* editor() const { return analyzer_->editor(); }
  sm::Factory* factory() const { return analyzer_->factory(); }
  CompilationSession* session() const { return analyzer_->session(); }

  void Run();

 private:
  sm::EnumMember* AnalyzeEnumMember(sm::Enum* enum_type,
                                    ast::EnumMember* ast_member,
                                    ast::EnumMember* ast_previous_member);
  sm::Type* EnsureEnumBase(ast::Enum* enum_type);
  void FixEnumMember(sm::EnumMember* member, sm::Value* value);

  sm::Semantic* SemanticOf(ast::Node* node) {
    return analyzer_->SemanticOf(node);
  }

  // ast::Visitor
  void VisitMethod(ast::Method* node) final;

  ClassAnalyzer* const analyzer_;
  std::unique_ptr<sm::Calculator> calculator_;

  DISALLOW_COPY_AND_ASSIGN(Resolver);
};

ClassAnalyzer::Resolver::Resolver(ClassAnalyzer* analyzer)
    : analyzer_(analyzer),
      calculator_(new sm::Calculator(analyzer->session())) {
}

// The entry point of |ClassAnalyzer|.
void ClassAnalyzer::Resolver::Run() {
  VisitNamespaceBody(session()->global_namespace_body());
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
  auto const method = factory()->NewMethod(method_group, signature, ast_method);
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
                     ast_method, other->ast_method());
  }
  // TODO(eval1749) Check whether |ast_method| overload methods in base class
  // with 'new', 'override' modifiers, or not
  // TODO(eval1749) Check |ast_method| not override static method.
}

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer
//
ClassAnalyzer::ClassAnalyzer(NameResolver* resolver) : Analyzer(resolver) {
}

ClassAnalyzer::~ClassAnalyzer() {
}

bool ClassAnalyzer::Run() {
  Collector(this).Run();
  if (!session()->errors().empty())
    return false;
  Resolver(this).Run();
  return session()->errors().empty();
}

void ClassAnalyzer::AddDependency(ast::Node* from, ast::Node* to) {
  dependency_graph_.AddEdge(from, to);
}

void ClassAnalyzer::Postpone(ast::Node* node) {
  pending_nodes_.push_back(node);
}

}  // namespace compiler
}  // namespace elang
