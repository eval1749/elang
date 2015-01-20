// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/method_resolver.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ir/nodes.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// MehtodResolver
//
MethodResolver::MethodResolver(NameResolver* name_resolver)
    : name_resolver_(name_resolver) {
}

MethodResolver::~MethodResolver() {
}

bool MethodResolver::IsApplicable(const ir::Method* method, int arity) {
  auto const signature = method->signature();
  return arity >= signature->minimum_arity() &&
         arity <= signature->maximum_arity();
}

// TODO(eval1749) We should pass to exclude `void` methods.
std::vector<ir::Method*> MethodResolver::ComputeApplicableMethods(
    ast::MethodGroup* method_group,
    ts::Value* output,
    int arity) {
  std::vector<ir::Method*> methods;
  // TODO(eval1749) We should check base classes.
  for (auto const ast_method : method_group->methods()) {
    auto const method = name_resolver()->Resolve(ast_method)->as<ir::Method>();
    DCHECK(method) << " Not resolved: " << *ast_method;
    if (!IsApplicable(method, arity))
      continue;
    methods.push_back(method);
  }
  return methods;
}

}  // namespace compiler
}  // namespace elang
