// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/lir/printer_generic.h"

#include "elang/lir/instructions.h"

namespace elang {
namespace lir {

GenericPrintableInstruction PrintAsGeneric(
    const Instruction* instruction) {
  return GenericPrintableInstruction(instruction);
}

GenericPrintableOpcode PrintAsGeneric(Opcode opcode) {
  return GenericPrintableOpcode(opcode);
}

GenericPrintableValue PrintAsGeneric(Value value) {
  return GenericPrintableValue(value);
}

std::ostream& operator<<(std::ostream& ostream,
                         const GenericPrintableValue& printable) {
  auto const value = printable.value;
  switch (value.kind) {
    case Value::Kind::Argument:
      return ostream << "arg[" << value.data << "]";
    case Value::Kind::Immediate:
      return ostream << "#" << value.data;
    case Value::Kind::Parameter:
      return ostream << "param[" << value.data << "]";
    case Value::Kind::PhysicalRegister:
      return ostream << (value.type == Value::Type::Float ? "F" : "R")
                     << value.data;
    case Value::Kind::VirtualRegister:
      return ostream << (value.type == Value::Type::Float ? "%f" : "%r")
                     << value.data;
    case Value::Kind::StackSlot:
      return ostream << "sp[" << value.data << "]";
  }
  return ostream << "UNSUPPORTED(" << value << ")";
}

std::ostream& operator<<(std::ostream& ostream,
                         const GenericPrintableOpcode& printable) {
  static const char* const mnemonics[] = {
#define V(Name, mnemonic, ...) mnemonic,
      FOR_EACH_LIR_INSTRUCTION(V)
#undef V
          "Invalid",
  };
  return ostream << mnemonics[std::min(static_cast<size_t>(printable.opcode),
                                       arraysize(mnemonics) - 1)];
}

std::ostream& operator<<(std::ostream& ostream,
                         const GenericPrintableInstruction& printable) {
  auto const instr = printable.instruction;
  ostream << PrintAsGeneric(instr->opcode());
  auto separator = " ";
  for (auto const output : instr->outputs()) {
    ostream << separator << PrintAsGeneric(output);
    separator = ", ";
  }
  ostream << " =";
  separator = " ";
  for (auto const input : instr->inputs()) {
    ostream << separator << PrintAsGeneric(input);
    separator = ", ";
  }
  return ostream;
}
}  // namespace lir
}  // namespace elang
