// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_FORMATTERS_TEXT_FORMATTER_H_
#define ELANG_OPTIMIZER_FORMATTERS_TEXT_FORMATTER_H_

#include <memory>
#include <ostream>

#include "base/macros.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
class ELANG_OPTIMIZER_EXPORT TextFormatter final {
 public:
  explicit TextFormatter(std::ostream* stream);
  ~TextFormatter();

  void FormatFunction(const Function* function);

 private:
  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_FORMATTERS_TEXT_FORMATTER_H_
