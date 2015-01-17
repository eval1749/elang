// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/method_resolver.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/analyze/type_evaluator.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ir/nodes.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// MehtodResolver
//
MethodResolver::MethodResolver(TypeEvaluator* type_evaluator)
    : type_evaluator_(type_evaluator) {
}

MethodResolver::~MethodResolver() {
}

NameResolver* MethodResolver::name_resolver() {
  return type_evaluator_->name_resolver();
}

bool MethodResolver::IsApplicable(const ir::Method* method,
                                  const std::vector<ts::Value*>& arguments) {
  auto const arity = static_cast<int>(arguments.size());
  auto const signature = method->signature();
  if (arity < signature->minimum_arity() || arity > signature->maximum_arity())
    return false;
  auto parameters = signature->parameters().begin();
  for (auto const argument : arguments) {
    auto const parameter = *parameters;
    auto const parameter_value = type_evaluator_->NewLiteral(parameter->type());
    if (!parameter_value->Contains(argument))
      return false;
    if (!parameter->is_rest())
      ++parameters;
  }
  return true;
}

std::unordered_set<ir::Method*> MethodResolver::Resolve(
    ast::MethodGroup* method_group,
    ts::Value* output,
    const std::vector<ts::Value*>& arguments) {
  std::unordered_set<ir::Method*> methods;
  for (auto const ast_method : method_group->methods()) {
    auto const method = name_resolver()->Resolve(ast_method)->as<ir::Method>();
    DCHECK(method) << " Not resolved: " << *ast_method;
    if (methods.count(method))
      continue;
    if (!IsApplicable(method, arguments))
      continue;
    methods.insert(method);
  }
  return methods;
}

}  // namespace compiler
}  // namespace elang
