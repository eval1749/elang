// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/lir/printer_generic.h"

#include "base/strings/string_piece.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

namespace {
base::StringPiece SizeSuffixOf(Value value) {
  static const char* const suffixes[] = {"b", "w", "", "l"};
  return suffixes[static_cast<int>(value.size)];
}

base::StringPiece TypeStringOf(Value value) {
  static const char* const types[] = {"i", "f"};
  return types[static_cast<int>(value.type)];
}
}  // namespace

GenericPrintableInstruction PrintAsGeneric(const Instruction* instruction) {
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
      ostream << "arg[" << value.data << "]";
      break;
    case Value::Kind::Condition:
      return ostream << "%b" << value.data;
    case Value::Kind::Immediate:
      ostream << "#" << value.data;
      break;
    case Value::Kind::Literal:
      ostream << "literal@" << value.data;
      break;
    case Value::Kind::Parameter:
      ostream << "param[" << value.data << "]";
      break;
    case Value::Kind::PhysicalRegister:
      ostream << (value.type == Value::Type::Float ? "f" : "r") << value.data;
      break;
    case Value::Kind::SpillSlot:
      ostream << "$" << TypeStringOf(value) << value.data;
      break;
    case Value::Kind::StackSlot:
      ostream << "sp[" << value.data << "]";
      break;
    case Value::Kind::VirtualRegister:
      ostream << (value.type == Value::Type::Float ? "%f" : "%r") << value.data;
      break;
    case Value::Kind::Void:
      return ostream << "void";
    default:
      ostream << "UNSUPPORTED(" << value << ")";
      return ostream;
  }
  return ostream << SizeSuffixOf(value);
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
  for (auto const block : instr->block_operands()) {
    ostream << separator << *block;
    separator = ", ";
  }
  return ostream;
}
}  // namespace lir
}  // namespace elang
