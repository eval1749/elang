// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/isa_x64.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_map.h"
#include "elang/lir/printable.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// AsPrintableValue
//
std::ostream& operator<<(std::ostream& ostream,
                         const AsPrintableValue& printable) {
  static const char* const names8[16] = {
      "AL",
      "CL",
      "BL",
      "SPL",
      "BPL",
      "SIL",
      "DIL",
      "R8L",
      "R9L",
      "R10L",
      "R11L",
      "R12L",
      "R13L",
      "R14L",
      "R15L",
  };
  static const char* const names16[16] = {
      "AX",
      "CX",
      "BX",
      "SP",
      "BP",
      "SI",
      "DI",
      "R8W",
      "R9W",
      "R10W",
      "R11W",
      "R12W",
      "R13W",
      "R14W",
      "R15W",
  };
  static const char* const names32[16] = {
      "EAX",
      "ECX",
      "EBX",
      "ESP",
      "EBP",
      "ESI",
      "EDI",
      "R8D",
      "R9D",
      "R10D",
      "R11D",
      "R12D",
      "R13D",
      "R14D",
      "R15D",
  };
  static const char* const names64[16] = {
      "RAX",
      "RCX",
      "RBX",
      "RSP",
      "RBP",
      "RSI",
      "RDI",
      "R8",
      "R9",
      "R10",
      "R11",
      "R12",
      "R13",
      "R14",
      "R15",
  };
  auto const value = printable.value;
  switch (value.kind) {
    case Value::Kind::Invalid:
      return ostream << "INVALID";
    case Value::Kind::GeneralRegister:
      if (value.data >= 0x300)
        return ostream << names64[value.data & 15];
      if (value.data >= 0x200)
        return ostream << names32[value.data & 15];
      if (value.data >= 0x100)
        return ostream << names16[value.data & 15];
      return ostream << names8[value.data & 15];
    case Value::Kind::FloatRegister:
      return ostream << "XMM" << value.data;
    case Value::Kind::Immediate:
      return ostream << value.data;
    case Value::Kind::Literal:
      if (printable.literals) {
        if (auto const literal = printable.literals->GetLiteral(value))
          return ostream << *literal;
      }
      return ostream << "#" << value.data;
    case Value::Kind::Parameter:
      return ostream << "%param[" << value.data << "]";
    case Value::Kind::VirtualGeneralRegister:
      return ostream << "%r" << value.data;
    case Value::Kind::VirtualFloatRegister:
      return ostream << "%f" << value.data;
  }
  return ostream << "?" << value.kind << "?" << value.data;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Instruction& instruction) {
  if (auto const block = instruction.basic_block())
    ostream << "bb:" << block->id();
  else
    ostream << "--:";
  ostream << instruction.id() << ":" << instruction.mnemonic();
  if (!instruction.outputs().empty()) {
    for (auto const output : instruction.outputs())
      ostream << " " << output;
    ostream << " <-";
  }
  for (auto const input : instruction.inputs())
    ostream << " " << input;
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const AsPrintableInstruction& printable) {
  auto const instruction = printable.instruction;
  auto const literals = printable.literals;
  ostream << instruction->mnemonic();
  auto separator = " ";
  // outputs
  if (!instruction->outputs().empty()) {
    auto const output = instruction->output(0);
    ostream << separator << AsPrintableValue(literals, output);
    separator = ", ";
  }
  // inputs
  if (!instruction->inputs().empty()) {
    for (auto const value : instruction->inputs()) {
      ostream << separator << AsPrintableValue(literals, value);
      separator = ", ";
    }
  }
  if (instruction->outputs().size() >= 2) {
    ostream << " ;";
    for (auto output : instruction->outputs())
      ostream << " " << output;
  }
  return ostream;
}

Value Isa::GetRegister(isa::Register name) {
  auto const number = static_cast<int>(name);
  if (number >= isa::XMM0 && number <= isa::XMM15)
    return Value(Value::Kind::FloatRegister, number);
  return Value(Value::Kind::GeneralRegister, number);
}

bool Isa::IsCopyable(Value output, Value input) {
  if (output.kind == Value::Kind::GeneralRegister) {
    if (input.kind == Value::Kind::GeneralRegister)
      return (output.data & ~15) == (input.data & ~15);
    return input.kind == Value::Kind::VirtualGeneralRegister;
  }
  if (output.kind == Value::Kind::VirtualGeneralRegister) {
    return input.kind == Value::Kind::GeneralRegister ||
           input.kind == Value::Kind::VirtualGeneralRegister;
  }
  if (output.kind == Value::Kind::FloatRegister) {
    return input.kind == Value::Kind::FloatRegister ||
           input.kind == Value::Kind::VirtualFloatRegister;
  }
  if (output.kind == Value::Kind::VirtualFloatRegister) {
    return input.kind == Value::Kind::FloatRegister ||
           input.kind == Value::Kind::VirtualFloatRegister;
  }
  NOTREACHED() << "Invalid output " << output;
  return false;
}

}  // namespace lir
}  // namespace elang
