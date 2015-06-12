// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>
#include <string>

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "base/strings/string_piece.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_map.h"
#include "elang/lir/printable.h"
#include "elang/lir/target_x64.h"

namespace elang {
namespace lir {

namespace {
Value AdjustTypeForCall(Value type) {
  return type.is_int8() || type.is_int16() ? Value::Int32Type() : type;
}
}

//////////////////////////////////////////////////////////////////////
//
// PrintableValue
//
std::ostream& operator<<(std::ostream& ostream,
                         const PrintableValue& printable) {
  static const char* const names8[16] = {
      "AL",
      "CL",
      "DL",
      "BL",
      "BPL",
      "SPL",
      "SIL",
      "DIL",
      "R8B",
      "R9B",
      "R10B",
      "R11B",
      "R12B",
      "R13B",
      "R14B",
      "R15B",
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
  static const char* const sizes[8] = {
      "8", "16", "32", "64", "?5", "?6", "0",
  };
  auto const value = printable.value;
  switch (value.kind) {
    case Value::Kind::Argument:
      return ostream << "%arg[" << value.data << "]";
    case Value::Kind::Conditional:
      if (value.data == 0)
        return ostream << "true";
      if (value.data == 1)
        return ostream << "false";
      return ostream << "%b" << value.data;
    case Value::Kind::FrameSlot:
      return ostream << "[rbp+" << value.data << "]";
    case Value::Kind::Immediate:
      if (value.is_64bit())
        return ostream << value.data << "l";
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
      if (value.is_int32())
        return ostream << names32[value.data];
      if (value.is_int64())
        return ostream << names64[value.data];
      if (value.is_int16())
        return ostream << names16[value.data];
      if (value.is_int8())
        return ostream << names8[value.data];
      if (value.is_float32())
        return ostream << "XMM" << value.data << "S";
      if (value.is_float64())
        return ostream << "XMM" << value.data << "D";
      break;
    case Value::Kind::Parameter:
      return ostream << "%param[" << value.data << "]";
    case Value::Kind::VirtualRegister:
      switch (value.size) {
        case ValueSize::Size8:
          return ostream << "%r" << value.data << "b";
        case ValueSize::Size16:
          return ostream << "%r" << value.data << "w";
        case ValueSize::Size32:
          return ostream << (value.is_float() ? "%f" : "%r") << value.data;
        case ValueSize::Size64:
          if (value.is_float())
            return ostream << "%f" << value.data << "d";
          return ostream << "%r" << value.data << "l";
      }
      NOTREACHED() << value.size;
      return ostream << "NOTREACHED(" << value.data << ")";
    case Value::Kind::SpillSlot:
      return ostream << "%spill[" << value.data << "]";
    case Value::Kind::StackSlot:
      return ostream << "%stack[" << value.data << "]";
    case Value::Kind::Void:
      if (value.size == ValueSize::Size0)
        return ostream << "void";
      return ostream << (value.is_integer() ? "int" : "float")
                     << sizes[static_cast<size_t>(value.size)];
  }
  NOTREACHED() << value.kind;
  return ostream << "NOTREACHED(" << value.data << ")";
}

namespace isa {
static const Register kIntegerParameters[] = {RCX, RDX, R8, R9};
static const Register kFloatParameters[] = {XMM0D, XMM1D, XMM2D, XMM3D};

static_assert(sizeof(kIntegerParameters) == sizeof(kFloatParameters),
              "Number of parameter registers should be matched between"
              " float and integer");
}

// All registers except for XMM0 are allocatable.
std::vector<Value> Target::AllocatableFloatRegisters() {
  std::vector<Value> registers(0);
  auto index = 0;
  for (auto mask = isa::kAllocatableFloatRegisters; mask; mask >>= 1) {
    if ((mask & 1) == 0)
      continue;
    registers.push_back(
        RegisterOf(static_cast<isa::Register>(isa::XMM0D + index)));
    ++index;
  }
  return registers;
}

std::vector<Value> Target::AllocatableGeneralRegisters() {
  std::vector<Value> registers(0);
  auto index = 0;
  for (auto mask = isa::kAllocatableGeneralRegisters; mask; mask >>= 1) {
    if ((mask & 1) == 0)
      continue;
    registers.push_back(
        RegisterOf(static_cast<isa::Register>(isa::RAX + index)));
    ++index;
  }
  return registers;
}

Value Target::ArgumentAt(Value output, size_t position) {
  auto const it = std::begin(isa::kIntegerParameters) + position;
  if (it < std::end(isa::kIntegerParameters)) {
    auto const number = output.is_float() ? isa::kFloatParameters[position]
                                          : isa::kIntegerParameters[position];
    return Value(output.type, output.size, Value::Kind::PhysicalRegister,
                 number & 15);
  }
  // TODO(eval1749) We should make |Value::Argument()| to take |size_t|.
  return Value::Argument(output, base::checked_cast<int>(position));
}

// We can use |MOV r/m, imm32| instruction.
bool Target::HasCopyImmediateToMemory(Value value) {
  if (value.is_float())
    return false;
  // TODO(eval1749) We should check literal map whether value is 32-bit integer
  // or not.
  return value.is_immediate() || Value::SizeOf(value) <= 4;
}

// For integer, we can use |XCHG r, r/m| instruction.
// Note: We should not use |XCHG| with memory operand, since is is slow and
// locks memory.
bool Target::HasSwapInstruction(Value value) {
  return value.is_integer();
}

bool Target::HasXorInstruction(Value value) {
  return true;
}

Value Target::IntPtrType() {
  return Value::Int64Type();
}

bool Target::IsCalleeSavedRegister(Value value) {
  DCHECK(value.is_physical());
  auto const mask = 1 << (value.data & 15);
  if (value.is_float())
    return (isa::kFloatCalleeSavedRegisters & mask) != 0;
  return (isa::kGeneralCalleeSavedRegisters & mask) != 0;
}

bool Target::IsCallerSavedRegister(Value value) {
  DCHECK(value.is_physical());
  auto const mask = 1 << (value.data & 15);
  if (value.is_float())
    return (isa::kFloatCallerSavedRegisters & mask) != 0;
  return (isa::kGeneralCallerSavedRegisters & mask) != 0;
}

bool Target::IsParameterRegister(Value value) {
  DCHECK(value.is_physical());
  auto const mask = 1 << (value.data & 15);
  if (value.is_float())
    return (isa::kFloatParameterRegisters & mask) != 0;
  return (isa::kGeneralParameterRegisters & mask) != 0;
}

Value Target::NaturalRegisterOf(Value physical) {
  DCHECK(physical.is_physical());
  return Value(physical.type, ValueSize::Size64, Value::Kind::PhysicalRegister,
               physical.data);
}

Value Target::ParameterAt(Value type, size_t position) {
  auto const output = AdjustTypeForCall(type);
  auto const it = std::begin(isa::kIntegerParameters) + position;
  if (it < std::end(isa::kIntegerParameters)) {
    auto const number = output.is_float() ? isa::kFloatParameters[position]
                                          : isa::kIntegerParameters[position];
    return Value(output.type, output.size, Value::Kind::PhysicalRegister,
                 number & 15);
  }
  // TODO(eval1749) We should make |Value::Parameter()| to take |size_t|.
  return Value::Parameter(output, base::checked_cast<int>(position));
}

Value Target::RegisterOf(isa::Register name) {
  auto const number = static_cast<int>(name);
  if (number >= isa::XMM0D && number <= isa::XMM15D) {
    return Value(Value::Type::Float, ValueSize::Size64,
                 Value::Kind::PhysicalRegister, number & 15);
  }
  if (number >= isa::XMM0S && number <= isa::XMM15S) {
    return Value(Value::Type::Float, ValueSize::Size32,
                 Value::Kind::PhysicalRegister, number & 15);
  }
  return Value(Value::Type::Integer, static_cast<ValueSize>(name >> 8),
               Value::Kind::PhysicalRegister, name & 15);
}

Value Target::ReturnAt(Value type, size_t position) {
  auto const return_type = AdjustTypeForCall(type);
  DCHECK_EQ(position, 0) << "NYI multiple return values";
  if (return_type.is_int32())
    return RegisterOf(isa::EAX);
  if (return_type.is_int64())
    return RegisterOf(isa::RAX);
  if (return_type.is_float32())
    return RegisterOf(isa::XMM0S);
  if (return_type.is_float64())
    return RegisterOf(isa::XMM0D);
  NOTREACHED() << "Unsupported return type: " << type;
  return Value::Void();
}

}  // namespace lir
}  // namespace elang
