// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/method_resolver.h"

#include "base/logging.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/analysis/type_values.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/semantics/nodes.h"

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

bool MethodResolver::IsApplicable(const sm::Method* method, size_t arity) {
  auto const signature = method->signature();
  return arity >= signature->minimum_arity() &&
         arity <= signature->maximum_arity();
}

// TODO(eval1749) We should pass to exclude `void` methods.
std::vector<sm::Method*> MethodResolver::ComputeApplicableMethods(
    sm::MethodGroup* method_group,
    ts::Value* output,
    size_t arity) {
  std::vector<sm::Method*> methods;
  // TODO(eval1749) We should check base classes.
  for (auto const method : method_group->methods()) {
    if (!IsApplicable(method, arity))
      continue;
    methods.push_back(method);
  }
  return methods;
}

}  // namespace compiler
}  // namespace elang
