// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FORMATTERS_TEXT_FORMATTER_H_
#define ELANG_HIR_FORMATTERS_TEXT_FORMATTER_H_

#include <memory>
#include <ostream>

#include "base/macros.h"

namespace elang {
namespace hir {

class Function;
class Instruction;
class Operand;

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
class TextFormatter final {
 public:
  explicit TextFormatter(std::ostream* stream);
  ~TextFormatter();

  void FormatFunction(const Function* function);
  std::ostream& FormatInstruction(const Instruction* instruction);

 private:
  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FORMATTERS_TEXT_FORMATTER_H_
