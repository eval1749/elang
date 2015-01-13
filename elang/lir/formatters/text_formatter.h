// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_FORMATTERS_TEXT_FORMATTER_H_
#define ELANG_LIR_FORMATTERS_TEXT_FORMATTER_H_

#include <memory>
#include <ostream>

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

class Factory;
class Function;
class Instruction;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
class ELANG_LIR_EXPORT TextFormatter final {
 public:
  explicit TextFormatter(Factory* factory, std::ostream* stream);
  ~TextFormatter();

  void FormatFunction(const Function* function);
  std::ostream& FormatInstruction(const Instruction* instruction);
  std::ostream& FormatValue(Value value);

 private:
  Factory* const factory_;
  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_FORMATTERS_TEXT_FORMATTER_H_
