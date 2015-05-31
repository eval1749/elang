// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analysis/class_analyzer.h"

#include "base/logging.h"
#include "elang/compiler/analysis/analyzer.h"
#include "elang/compiler/analysis/const_expr_analyzer.h"
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
class Collector final : public ast::Visitor {
 public:
  explicit Collector(ConstExprAnalyzer* analyzer);
  ~Collector() = default;

  void Run();

 private:
  CompilationSession* session() const { return analyzer_->session(); }

  sm::Type* EnsureEnumBase(ast::Enum* enum_type);

  // ast::Visitor
  void VisitEnum(ast::Enum* node) final;
  void VisitField(ast::Field* node) final;

  ConstExprAnalyzer* const analyzer_;

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

Collector::Collector(ConstExprAnalyzer* analyzer) : analyzer_(analyzer) {
}

sm::Type* Collector::EnsureEnumBase(ast::Enum* enum_type) {
  auto const type =
      enum_type->enum_base()
          ? analyzer_->ResolveTypeReference(enum_type->enum_base(), enum_type)
          : session()->PredefinedTypeOf(PredefinedName::Int32);
  if (analyzer_->calculator()->IsIntType(type))
    return type;
  DCHECK(enum_type->enum_base()) << enum_type;
  analyzer_->Error(ErrorCode::SemanticEnumEnumBase, enum_type->enum_base());
  return session()->PredefinedTypeOf(PredefinedName::Int64);
}

// The entry point of |Collector|.
void Collector::Run() {
  Traverse(session()->global_namespace_body());
}

// ast::Visitor
void Collector::VisitEnum(ast::Enum* ast_enum) {
  auto const enum_base = EnsureEnumBase(ast_enum);
  auto const enum_type = analyzer_->SemanticOf(ast_enum)->as<sm::Enum>();
  analyzer_->editor()->FixEnumBase(enum_type, enum_base);
  for (auto const ast_node : ast_enum->members())
    analyzer_->AnalyzeEnumMember(ast_node->as<ast::EnumMember>());
}

void Collector::VisitField(ast::Field* node) {
  DCHECK(node);
}

//////////////////////////////////////////////////////////////////////
//
// Resolver
//
class Resolver final : public ast::Visitor {
 public:
  explicit Resolver(ConstExprAnalyzer* analyzer);
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

  ConstExprAnalyzer* const analyzer_;

  DISALLOW_COPY_AND_ASSIGN(Resolver);
};

Resolver::Resolver(ConstExprAnalyzer* analyzer) : analyzer_(analyzer) {
}

// The entry point of |Resolver|.
void Resolver::Run() {
  Traverse(session()->global_namespace_body());
}

// ast::Visitor
void Resolver::VisitMethod(ast::Method* ast_method) {
  auto const class_body = ast_method->parent()->as<ast::ClassBody>();
  DCHECK(class_body) << ast_method;
  auto const clazz = SemanticOf(class_body);
  auto const method_group =
      clazz->FindMember(ast_method->name())->as<sm::MethodGroup>();
  DCHECK(method_group) << ast_method << " "
                       << clazz->FindMember(ast_method->name());
  auto const return_type =
      analyzer_->ResolveTypeReference(ast_method->return_type(), ast_method);
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
  // TODO(eval1749) Calculate value of default parameters
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer
//
ClassAnalyzer::ClassAnalyzer(NameResolver* resolver)
    : editor_(new sm::Editor(resolver->session())),
      analyzer_(new ConstExprAnalyzer(resolver, editor_.get())) {
}

ClassAnalyzer::~ClassAnalyzer() {
}

void ClassAnalyzer::Run() {
  Collector(analyzer_.get()).Run();
  if (analyzer_->session()->HasError())
    return;
  analyzer_->Run();
  if (analyzer_->session()->HasError())
    return;
  Resolver(analyzer_.get()).Run();
}

}  // namespace compiler
}  // namespace elang
