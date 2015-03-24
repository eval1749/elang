// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_FORMATTERS_TEXT_FORMATTER_H_
#define ELANG_OPTIMIZER_FORMATTERS_TEXT_FORMATTER_H_

#include <ostream>

#include "base/macros.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class Function;

struct ELANG_OPTIMIZER_EXPORT AsReversePostOrder {
  const Function* function;
  explicit AsReversePostOrder(const Function* function) : function(function) {}
};

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(
    std::ostream& ostream,
    const AsReversePostOrder& thing);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_FORMATTERS_TEXT_FORMATTER_H_
