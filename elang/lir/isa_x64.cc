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
    case Value::Kind::Argument:
      return ostream << "%arg[" << value.data << "]";
    case Value::Kind::Condition:
      return ostream << "EFLAGS";
    case Value::Kind::Immediate:
      return ostream << value.data;
    case Value::Kind::Instruction:
      if (printable.literals) {
        if (auto const instruction = printable.literals->GetInstruction(value))
          return ostream << *instruction;
      }
      return ostream << "#" << value.data;
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
    case Value::Kind::Void:
      return ostream << "void";
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
    auto separator = "";
    for (auto const output : instruction.outputs()) {
      ostream << separator << output;
      separator = ", ";
    }
    ostream << " =";
  }
  if (auto const phi = instruction.as<PhiInstruction>()) {
    auto separator = " ";
    for (auto const phi_input : phi->phi_inputs()) {
      ostream << separator << *phi_input->basic_block() << " "
              << phi_input->value();
      separator = ", ";
    }
    return ostream;
  }
  auto separator = " ";
  for (auto const input : instruction.inputs()) {
    ostream << separator << input;
    separator = ", ";
  }
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const AsPrintableInstruction& printable) {
  auto const instruction = printable.instruction;
  auto const literals = printable.literals;
  ostream << instruction->mnemonic();
  // outputs
  if (!instruction->outputs().empty()) {
    auto separator = " ";
    for (auto output : instruction->outputs()) {
      ostream << separator << output;
      separator = ", ";
    }
    ostream << " =";
  }
  // inputs
  if (auto const phi = instruction->as<PhiInstruction>()) {
    auto separator = " ";
    for (auto const phi_input : phi->phi_inputs()) {
      ostream << separator << *phi_input->basic_block() << " "
              << AsPrintableValue(literals, phi_input->value());
      separator = ", ";
    }
    DCHECK_EQ(phi->outputs().size(), 1);
    return ostream;
  }
  if (!instruction->inputs().empty()) {
    auto separator = " ";
    for (auto const value : instruction->inputs()) {
      ostream << separator << AsPrintableValue(literals, value);
      separator = ", ";
    }
  }
  return ostream;
}

namespace isa {
static const Register kIntegerParameters[] = {RCX, RDX, R8, R9};
static const Register kFloatParameters[] = {XMM0D, XMM1D, XMM2D, XMM3D};

static_assert(sizeof(kIntegerParameters) == sizeof(kFloatParameters),
              "Number of parameter registers should be matched between"
              " float and integer");
}

Value Isa::GetArgumentAt(Value output, int position) {
  DCHECK_GE(position, 0);
  if (position < static_cast<int>(arraysize(isa::kIntegerParameters))) {
    auto const number = output.is_float() ? isa::kFloatParameters[position]
                                          : isa::kIntegerParameters[position];
    return Value(output.type, output.size, Value::Kind::PhysicalRegister,
                 number & 15);
  }
  return Value(output.type, Value::Size::Size64, Value::Kind::Argument,
               position);
}

Value Isa::GetParameterAt(Value output, int position) {
  DCHECK_GE(position, 0);
  if (position < static_cast<int>(arraysize(isa::kIntegerParameters))) {
    auto const number = output.is_float() ? isa::kFloatParameters[position]
                                          : isa::kIntegerParameters[position];
    return Value(output.type, output.size, Value::Kind::PhysicalRegister,
                 number & 15);
  }
  return Value(output.type, Value::Size::Size64, Value::Kind::Parameter,
               position);
}

Value Isa::GetRegister(isa::Register name) {
  if (name == isa::EFLAGS) {
    return Value(Value::Type::Integer, Value::Size::Size32,
                 Value::Kind::Condition, 0);
  }
  auto const number = static_cast<int>(name);
  if (number >= isa::XMM0D && number <= isa::XMM15D) {
    return Value(Value::Type::Float, Value::Size::Size64,
                 Value::Kind::PhysicalRegister, number & 15);
  }
  if (number >= isa::XMM0S && number <= isa::XMM15S) {
    return Value(Value::Type::Float, Value::Size::Size32,
                 Value::Kind::PhysicalRegister, number & 15);
  }
  return Value(Value::Type::Integer, static_cast<Value::Size>(name >> 8),
               Value::Kind::PhysicalRegister, name & 15);
}

Value::Size Isa::PointerSize() {
  return Value::Size::Size64;
}

}  // namespace lir
}  // namespace elang
