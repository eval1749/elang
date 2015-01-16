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
class Method;
class MethodGroup;
}

namespace ts {
class Value;
}

//////////////////////////////////////////////////////////////////////
//
// MethodResolver
//
class MethodResolver final {
 public:
  MethodResolver();
  ~MethodResolver();

  std::unordered_set<ast::Method*> Resolve(
      ast::MethodGroup* method_group,
      ts::Value* output,
      const std::vector<ts::Value*>& argument);

 private:
  DISALLOW_COPY_AND_ASSIGN(MethodResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_METHOD_RESOLVER_H_
