// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "base/logging.h"
#include "elang/targets/register_x64.h"

namespace elang {
namespace targets {
namespace x64 {

int RegisterKind(Register reg) {
  return static_cast<int>(reg) & 0xFF00;
}

Register RegisterOf(Register reg, int index) {
  DCHECK_EQ(static_cast<int>(reg) & 0xFF, 0);
  return static_cast<Register>(static_cast<int>(reg) + index);
}

Register RegisterOf(OperandSize size, int name) {
  DCHECK_GE(name, 0);
  DCHECK_LE(name, 255);
  switch (size) {
    case OperandSize::Is8:
      return RegisterOf(Register::AL, name);
    case OperandSize::Is16:
      return RegisterOf(Register::AX, name);
    case OperandSize::Is32:
      return RegisterOf(Register::EAX, name);
    case OperandSize::Is64:
      return RegisterOf(Register::RAX, name);
    case OperandSize::Is128:
      return RegisterOf(Register::XMM0, name);
    case OperandSize::Is256:
      return RegisterOf(Register::XMM1, name);
  }
  NOTREACHED();
  return Register::None;
}

OperandSize RegisterSizeOf(Register reg) {
  if (reg == Register::None)
    return OperandSize::Is0;
  auto const type = static_cast<int>(reg) & 0xFFF00;
  if (type == static_cast<int>(Register::RAX))
    return OperandSize::Is64;
  if (type == static_cast<int>(Register::EAX))
    return OperandSize::Is32;
  if (type == static_cast<int>(Register::AX))
    return OperandSize::Is16;
  if (type == static_cast<int>(Register::AL))
    return OperandSize::Is16;
  if (type == static_cast<int>(Register::XMM0))
    return OperandSize::Is128;
  if (type == static_cast<int>(Register::YMM0))
    return OperandSize::Is256;
  if (type == static_cast<int>(Register::CS))
    return OperandSize::Is16;
  return OperandSize::Is0;
}

std::ostream& operator<<(std::ostream& ostream, Register reg) {
  static const char* reg64s[] = {"RAX",
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
                                 "RIP"};
  static const char* reg32s[] = {"EAX",
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
                                 "R15D"};
  static const char* reg16s[] = {"AX",
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
                                 "R15W"};
  static const char* reg8s[] = {"AL",   "CL",   "DL",   "BL",   "SPL",
                                "BPL",  "SIL",  "DIL",  "R8B",  "R9B",
                                "R10B", "R11B", "R12B", "R13B", "R14B",
                                "R15B", "AH",   "CH",   "DH",   "BH"};
  static const char* segments[] = {"CS", "DS", "ES", "FG", "GS"};

  if (reg == Register::None)
    return ostream << "None";
  auto const kind = RegisterKind(reg);
  auto const index = static_cast<int>(reg) & 0xFF;
  if (kind == RegisterKind(Register::RAX))
    return ostream << reg64s[index];
  if (kind == RegisterKind(Register::EAX))
    return ostream << reg32s[index];
  if (kind == RegisterKind(Register::AX))
    return ostream << reg16s[index];
  if (kind == RegisterKind(Register::AL))
    return ostream << reg8s[index];
  if (kind == RegisterKind(Register::XMM0))
    return ostream << "XMM" << index;
  if (kind == RegisterKind(Register::YMM0))
    return ostream << "YMM" << index;
  if (kind == RegisterKind(Register::CS))
    return ostream << segments[index];
  return ostream << "???";
}

}  // namespace x64
}  // namespace targets
}  // namespace elang
