// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ISA_X64_H_
#define ELANG_LIR_ISA_X64_H_

namespace elang {
namespace lir {
namespace isa {

// V0 opcode mnemonic
// V1 opcode mnemonic format
// V3 opcode mnemonic format1 format2
// VX opcode extension
//
#define FOR_EACH_X64_INSTRUCTION(V0, V1, V2, VX) \
  /* 0x70 */                                     \
  V1(0x70, jo, Jb)                               \
  V1(0x71, jno, Jb)                              \
  V1(0x72, jb, Jb)                               \
  V1(0x73, jae, Jb)                              \
  V1(0x74, je, Jb)                               \
  V1(0x75, jne, Jb)                              \
  V2(0x8B, mov, Gv, Ev)                          \
  /* 0x90 */                                     \
  V0(0x90, nop)                                  \
  /* 0xC0 */                                     \
  V0(0xC3, ret)                                  \
  /* 0xE0 */                                     \
  V1(0xE8, call, Jv)                             \
  V1(0xE9, jmp, Jv)

enum class Opcode {
#define V0(opcode, mnemonic) mnemonic = opcode,
#define V1(opcode, mnemonic, format) mnemonic##_##format = opcode,
#define V2(opcode, mnemonic, format1, format2) \
  mnemonic##_##format1##_##format2 = opcode,
#define VX(opcode, format) ext_##format = opcode,
  FOR_EACH_X64_INSTRUCTION(V0, V1, V2, VX)
#undef V0
#undef V1
#undef V2
#undef VX
};

// Calling convention
// First 4 parameters:
//  RCX, RDX, R8, R9, others passed on stack
//  XMM0, XMM1, XMM2, XMM3

//  HIGH    +--------------+
//  RSP-40  | parameter[6] |
//          +--------------+
//  RSP-32  | parameter[5] |
//          +--------------+
//  RSP-28  | parameter[4] |
//          +--------------+
//  RSP-24  | parameter[3] | R9/XMM3
//          +--------------+
//  RSP-20  | parameter[2] | R8/XMM2
//          +--------------+
//  RSP-16  | parameter[1] | RDX/XMM1
//          +--------------+
//  RSP-12  | parameter[0] | RCX/XMM0
//          +--------------+
//  RSP-8   | return IP    |
//          +--------------+
//  RSP     | RBP          |
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

}  // namespace isa
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ISA_X64_H_
