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

struct PrintableInstruction {
  const Instruction* instruction;
  const LiteralMap* literals;

  explicit PrintableInstruction(const Instruction* instruction)
      : instruction(instruction), literals(nullptr) {}

  PrintableInstruction(const LiteralMap* literals,
                       const Instruction* instruction)
      : instruction(instruction), literals(literals) {}
};

struct PrintableValue {
  const LiteralMap* literals;
  Value value;

  explicit PrintableValue(Value value) : literals(nullptr), value(value) {}

  PrintableValue(const LiteralMap* literals, Value value)
      : literals(literals), value(value) {}
};

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableInstruction& printable);
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableValue& printable);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_PRINTABLE_H_
