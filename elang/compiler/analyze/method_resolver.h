// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_METHOD_RESOLVER_H_
#define ELANG_COMPILER_ANALYZE_METHOD_RESOLVER_H_

#include <vector>
#include <unordered_set>

#include "base/macros.h"

namespace elang {
namespace compiler {

namespace ast {
class MethodGroup;
}

namespace ir {
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
  explicit MethodResolver(TypeEvaluator* type_evaluator);
  ~MethodResolver();

  std::unordered_set<ir::Method*> Resolve(
      ast::MethodGroup* method_group,
      ts::Value* output,
      const std::vector<ts::Value*>& argument);

 private:
  NameResolver* name_resolver();

  // Returns true if |method| is applicable with |arguments|.
  bool IsApplicable(const ir::Method* method,
                    const std::vector<ts::Value*>& arguments);

  TypeEvaluator* const type_evaluator_;

  DISALLOW_COPY_AND_ASSIGN(MethodResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_METHOD_RESOLVER_H_
