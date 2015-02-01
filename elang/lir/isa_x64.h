// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ISA_X64_H_
#define ELANG_LIR_ISA_X64_H_

#include "elang/lir/opcodes_x64.h"

namespace elang {
namespace lir {
namespace isa {

// Calling convention
// First 4 parameters:
//  RCX, RDX, R8, R9, others passed on stack
//  XMM0, XMM1, XMM2, XMM3

//  HIGH    +--------------+
//  RSP+56  | parameter[6] |
//          +--------------+
//  RSP+48  | parameter[5] |
//          +--------------+
//  RSP+40  | parameter[4] |
//          +--------------+
//  RSP+32  | home[3]      | R9/XMM3
//          +--------------+
//  RSP+24  | home[2]      | R8/XMM2
//          +--------------+
//  RSP+16  | home[1]      | RDX/XMM1
//          +--------------+
//  RSP+8   | home[0]      | RCX/XMM0
//          +--------------+
//  RSP     | return IP    |
//          +--------------+
//  RSP     | callee save  |
//          +--------------+

enum Register {
  // 64-bit register
  RAX = 0x300,
  RCX = 0x301,
  RDX = 0x302,
  RBX = 0x303,
  RSP = 0x304,
  RBP = 0x305,
  RSI = 0x306,
  RDI = 0x307,
  R8 = 0x308,
  R9 = 0x309,
  R10 = 0x30A,
  R11 = 0x30B,
  R12 = 0x30C,
  R13 = 0x30D,
  R14 = 0x30E,
  R15 = 0x30F,

  // 32-bit register
  EAX = 0x200,
  ECX = 0x201,
  EDX = 0x202,
  EBX = 0x203,
  ESP = 0x204,
  EBP = 0x205,
  ESI = 0x206,
  EDI = 0x207,
  R8D = 0x208,
  R9D = 0x209,
  R10D = 0x20A,
  R11D = 0x20B,
  R12D = 0x20C,
  R13D = 0x20D,
  R14D = 0x20E,
  R15D = 0x20F,

  // 16-bit register
  AX = 0x100,
  CX = 0x101,
  DX = 0x102,
  BX = 0x103,
  SP = 0x104,
  BP = 0x105,
  SI = 0x106,
  DI = 0x107,
  R8W = 0x108,
  R9W = 0x109,
  R10W = 0x10A,
  R11W = 0x10B,
  R12W = 0x10C,
  R13W = 0x10D,
  R14W = 0x10E,
  R15W = 0x10F,

  // 16-bit register
  AL = 0x00,
  CL = 0x01,
  DL = 0x02,
  BL = 0x03,
  SPL = 0x04,
  BPL = 0x05,
  SIL = 0x06,
  DIL = 0x07,
  R8L = 0x08,
  R9L = 0x09,
  R10L = 0x0A,
  R11L = 0x0B,
  R12L = 0x0C,
  R13L = 0x0D,
  R14L = 0x0E,
  R15L = 0x0F,

  XMM0 = 0x10,
  XMM1 = 0x11,
  XMM2 = 0x12,
  XMM3 = 0x13,
  XMM4 = 0x14,
  XMM5 = 0x15,
  XMM6 = 0x16,
  XMM7 = 0x17,
  XMM8 = 0x18,
  XMM9 = 0x19,
  XMM10 = 0x1A,
  XMM11 = 0x1B,
  XMM12 = 0x1C,
  XMM13 = 0x1D,
  XMM14 = 0x1E,
  XMM15 = 0x1F,
};

const int kCallerSaveRegisters = RAX | RCX | RDX | R8 | R9 | R10 | R11;
const int kCalleeSaveRegisters = RBX | RDI | RSI | RSP | R12 | R13 | R14 | R15;

static_assert((kCalleeSaveRegisters | kCalleeSaveRegisters) == 0x30F,
              "caller and callee registers must contain all registers");

}  // namespace isa
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ISA_X64_H_
