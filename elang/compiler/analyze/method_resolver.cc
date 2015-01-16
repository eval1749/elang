// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/method_resolver.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// MehtodResolver
//
MethodResolver::MethodResolver() {
}

MethodResolver::~MethodResolver() {
}

std::unordered_set<ast::Method*> MethodResolver::Resolve(
    ast::MethodGroup* method_group,
    ts::Value* output,
    const std::vector<ts::Value*>& argument) {
  // TODO(eval1749) NYI MethodResolver::Resolve
  return std::unordered_set<ast::Method*>();
}

}  // namespace compiler
}  // namespace elang
