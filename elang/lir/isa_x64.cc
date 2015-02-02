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
      "DL",
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
      "DX",
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
      "EDX",
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
      "RDX",
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
    case Value::Kind::Immediate:
      return ostream << value.data;
    case Value::Kind::Literal:
      if (printable.literals) {
        if (auto const literal = printable.literals->GetLiteral(value))
          return ostream << *literal;
      }
      return ostream << "#" << value.data;
    case Value::Kind::PhysicalRegister:
      DCHECK_GE(value.data, 0);
      DCHECK_LT(value.data, 16);
      if (value.type == Value::Type::Float)
        return ostream << "XMM" << value.data;
      switch (value.size) {
        case Value::Size::Size8:
          return ostream << names8[value.data];
        case Value::Size::Size16:
          return ostream << names16[value.data];
        case Value::Size::Size32:
          return ostream << names32[value.data];
        case Value::Size::Size64:
          return ostream << names64[value.data];
      }
      break;
    case Value::Kind::Parameter:
      return ostream << "%param[" << value.data << "]";
    case Value::Kind::VirtualRegister:
      return ostream << (value.is_float() ? "%f" : "%r") << value.data;
  }
  NOTREACHED() << value;
  return ostream << "NOTREACHED(" << value.data << ")";
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
  if (number >= isa::XMM0 && number <= isa::XMM15) {
    return Value(Value::Type::Float, Value::Size::Size64,
                 Value::Kind::PhysicalRegister, number);
  }
  return Value(Value::Type::Integer, static_cast<Value::Size>(name >> 8),
               Value::Kind::PhysicalRegister, name & 15);
}

bool Isa::IsCopyable(Value output, Value input) {
  if (output.type != input.type || output.size != input.size)
    return false;
  if (output.is_register())
    return true;
  return input.is_register();
}

Value::Size Isa::PointerSize() {
  return Value::Size::Size64;
}

}  // namespace lir
}  // namespace elang
