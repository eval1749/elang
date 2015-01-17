// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/class_analyzer.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer
//
ClassAnalyzer::ClassAnalyzer(NameResolver* resolver) : Analyzer(resolver) {
}

ClassAnalyzer::~ClassAnalyzer() {
}

// The entry point of |ClassAnalyzer|.
bool ClassAnalyzer::Run() {
  VisitNamespaceBody(session()->root_node());
  return session()->errors().empty();
}

// ast::Visitor
void ClassAnalyzer::VisitClassBody(ast::ClassBody* node) {
  node->AcceptForMembers(this);
}

void ClassAnalyzer::VisitEnum(ast::Enum* node) {
  DCHECK(node);
}

void ClassAnalyzer::VisitField(ast::Field* node) {
  DCHECK(node);
}

void ClassAnalyzer::VisitMethod(ast::Method* ast_method) {
  auto const return_type =
      ResolveTypeReference(ast_method->return_type(), ast_method->owner());
  std::vector<ir::Parameter*> parameters(ast_method->parameters().size());
  parameters.resize(0);
  auto is_valid = return_type;
  for (auto const parameter : ast_method->parameters()) {
    auto const parameter_type =
        ResolveTypeReference(parameter->type(), ast_method);
    if (!parameter_type) {
      is_valid = false;
      continue;
    }
    parameters.push_back(factory()->NewParameter(ir::ParameterKind::Required,
                                                 parameter->name(),
                                                 parameter_type, nullptr));
  }
  if (!is_valid)
    return;

  auto const signature = factory()->NewSignature(return_type, parameters);

  // Check this size with existing signatures
  for (auto ast_other : ast_method->method_group()->methods()) {
    auto const other = Resolve(ast_other)->as<ir::Method>();
    if (!other)
      continue;
    if (!other->signature()->IsIdenticalParameters(signature))
      continue;
    Error(other->return_type() == return_type
              ? ErrorCode::ClassResolutionMethodDuplicate
              : ErrorCode::ClassResolutionMethodConflict,
          ast_method, ast_other);
    is_valid = false;
  }
  if (!is_valid)
    return;

  // TODO(eval1749) Check whether |ast_method| overload methods in base class
  // with 'new', 'override' modifiers, or not
  // TODO(eval1749) Check |ast_method| not override static method.

  auto const method = factory()->NewMethod(ast_method, signature);
  resolver()->DidResolve(ast_method, method);
}

void ClassAnalyzer::VisitNamespaceBody(ast::NamespaceBody* node) {
  node->AcceptForMembers(this);
}

}  // namespace compiler
}  // namespace elang
