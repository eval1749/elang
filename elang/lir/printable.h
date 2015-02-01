// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_PRINTABLE_H_
#define ELANG_LIR_PRINTABLE_H_

#include <ostream>

#include "elang/lir/value.h"

namespace elang {
namespace lir {

class Instruction;
class LiteralMap;

struct AsPrintableInstruction {
  const Instruction* instruction;
  const LiteralMap* literals;

  explicit AsPrintableInstruction(const Instruction* instruction)
      : instruction(instruction), literals(nullptr) {}

  AsPrintableInstruction(const LiteralMap* literals,
                         const Instruction* instruction)
      : instruction(instruction), literals(literals) {}
};

struct AsPrintableValue {
  const LiteralMap* literals;
  Value value;

  explicit AsPrintableValue(Value value) : literals(nullptr), value(value) {}

  AsPrintableValue(const LiteralMap* literals, Value value)
      : literals(literals), value(value) {}
};

std::ostream& operator<<(std::ostream& ostream,
                         const AsPrintableInstruction& printable);
std::ostream& operator<<(std::ostream& ostream,
                         const AsPrintableValue& printable);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_PRINTABLE_H_
