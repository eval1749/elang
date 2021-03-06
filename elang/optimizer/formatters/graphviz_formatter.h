// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_FORMATTERS_GRAPHVIZ_FORMATTER_H_
#define ELANG_OPTIMIZER_FORMATTERS_GRAPHVIZ_FORMATTER_H_

#include <ostream>

#include "base/macros.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class Function;

struct AsGraphvizFunction {
  const Function* function;
  explicit AsGraphvizFunction(const Function* function) : function(function) {}
};

std::ostream& operator<<(std::ostream& ostream,
                         const AsGraphvizFunction& thing);
AsGraphvizFunction AsGraphviz(const Function* function);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_FORMATTERS_GRAPHVIZ_FORMATTER_H_
