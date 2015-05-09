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
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer
//
ClassAnalyzer::ClassAnalyzer(NameResolver* resolver)
    : Analyzer(resolver), editor_(new sm::Editor(session())) {
}

ClassAnalyzer::~ClassAnalyzer() {
}

// The entry point of |ClassAnalyzer|.
bool ClassAnalyzer::Run() {
  VisitNamespaceBody(session()->global_namespace_body());
  return session()->errors().empty();
}

// ast::Visitor
void ClassAnalyzer::VisitEnum(ast::Enum* ast_enum) {
  auto const enum_base =
      name_resolver()
          ->ResolvePredefinedType(ast_enum->token(), PredefinedName::Int32)
          ->as<sm::Type>();
  auto const outer = SemanticOf(ast_enum->parent());
  auto const enum_type = factory()->NewEnum(outer, ast_enum->name(), enum_base);
  SetSemanticOf(ast_enum, enum_type);
  editor_->AddMember(outer, enum_type);
  std::vector<sm::EnumMember*> members;
  for (auto const ast_member : ast_enum->members())
    members.push_back(factory()->NewEnumMember(enum_type, ast_member->name()));
  editor_->FixEnum(enum_type, members);
}

void ClassAnalyzer::VisitField(ast::Field* node) {
  DCHECK(node);
}

void ClassAnalyzer::VisitMethod(ast::Method* ast_method) {
  auto const return_type =
      ResolveTypeReference(ast_method->return_type(), ast_method->owner());
  std::vector<sm::Parameter*> parameters(ast_method->parameters().size());
  parameters.resize(0);
  for (auto const parameter : ast_method->parameters()) {
    auto const parameter_type =
        ResolveTypeReference(parameter->type(), ast_method);
    parameters.push_back(
        factory()->NewParameter(parameter, parameter_type, nullptr));
  }

  auto const clazz = SemanticOf(ast_method->owner())->as<sm::Class>();
  auto const method_name = ast_method->name();
  auto const method_group = editor_->EnsureMethodGroup(clazz, method_name);
  auto const signature = factory()->NewSignature(return_type, parameters);
  auto const method = factory()->NewMethod(method_group, signature, ast_method);
  SetSemanticOf(ast_method, method);

  // Check this size with existing signatures
  for (auto other : method_group->methods()) {
    if (!other->signature()->IsIdenticalParameters(signature))
      continue;
    Error(other->return_type() == return_type
              ? ErrorCode::ClassResolutionMethodDuplicate
              : ErrorCode::ClassResolutionMethodConflict,
          ast_method, other->ast_method());
  }
  // TODO(eval1749) Check whether |ast_method| overload methods in base class
  // with 'new', 'override' modifiers, or not
  // TODO(eval1749) Check |ast_method| not override static method.
}

}  // namespace compiler
}  // namespace elang
