// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FORMATTERS_TEXT_FORMATTER_H_
#define ELANG_HIR_FORMATTERS_TEXT_FORMATTER_H_

#include <memory>
#include <ostream>

#include "base/macros.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

class Function;
class Instruction;
class Value;

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
class ELANG_HIR_EXPORT TextFormatter final {
 public:
  explicit TextFormatter(std::ostream* stream);
  ~TextFormatter();

  void FormatFunction(const Function* function);

 private:
  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FORMATTERS_TEXT_FORMATTER_H_
