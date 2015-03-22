// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_METHOD_RESOLVER_H_
#define ELANG_COMPILER_ANALYSIS_METHOD_RESOLVER_H_

#include <vector>

#include "base/macros.h"

namespace elang {
namespace compiler {

namespace ast {
class MethodGroup;
}

namespace sm {
class Method;
}

namespace ts {
class Value;
}

class NameResolver;
class TypeEvaluator;

//////////////////////////////////////////////////////////////////////
//
// MethodResolver
//
class MethodResolver final {
 public:
  explicit MethodResolver(NameResolver* name_resolver);
  ~MethodResolver();

  std::vector<sm::Method*> ComputeApplicableMethods(
      ast::MethodGroup* method_group,
      ts::Value* output,
      int arity);

 private:
  NameResolver* name_resolver() const { return name_resolver_; }

  // Returns true if |method| is applicable with |arity|.
  bool IsApplicable(const sm::Method* method, int arity);

  NameResolver* const name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(MethodResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_METHOD_RESOLVER_H_
