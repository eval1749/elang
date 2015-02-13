// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_PRINTER_GENERIC_H_
#define ELANG_LIR_PRINTER_GENERIC_H_

#include <ostream>

#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {
class Instruction;
enum class Opcode;

struct ELANG_LIR_EXPORT GenericPrintableInstruction final {
  const Instruction* instruction;

  explicit GenericPrintableInstruction(const Instruction* instruction)
      : instruction(instruction) {}
};

struct ELANG_LIR_EXPORT GenericPrintableOpcode final {
  Opcode opcode;
  explicit GenericPrintableOpcode(Opcode opcode) : opcode(opcode) {}
};

struct ELANG_LIR_EXPORT GenericPrintableValue final {
  Value value;
  explicit GenericPrintableValue(Value value) : value(value) {}
};

ELANG_LIR_EXPORT GenericPrintableInstruction PrintAsGeneric(
    const Instruction* instruction);
ELANG_LIR_EXPORT GenericPrintableOpcode PrintAsGeneric(Opcode opcode);
ELANG_LIR_EXPORT GenericPrintableValue PrintAsGeneric(Value value);

ELANG_LIR_EXPORT std::ostream& operator<<(
    std::ostream& ostream,
    const GenericPrintableInstruction& printable);

ELANG_LIR_EXPORT std::ostream& operator<<(
    std::ostream& ostream,
    const GenericPrintableOpcode& printable);

ELANG_LIR_EXPORT std::ostream& operator<<(
    std::ostream& ostream,
    const GenericPrintableValue& printable);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_PRINTER_GENERIC_H_
