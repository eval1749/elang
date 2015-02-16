// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ISA_X64_H_
#define ELANG_LIR_ISA_X64_H_

namespace elang {
namespace lir {

namespace isa {

// Instruction Format
//  legacy prefix
//  REX prefix
//  Opcode 1, 2 or 3 byte
//  ModRm 1 byte
//  SIB 1 byte
//  Displacement 1, 2, or 4 byte
//  Immediate 1, 2,, 4 or 8 byte

//    7 6  5 4 3  2 1 0    7 6 5 4 3 2 1 0
//   +----+------+------+ +---+-----+------+ +----------------+
//   |mod | reg  |  r/m | |SS | idx | base | |  disp8/disp32  |
//   +----+------+------+ +----------------+ +----------------+
//
//   mod=00 r/m=100 SIB
//   mod=01 r/m=100 SIB + disp8
//   mod=10 r/m=100 SIB + disp32
//
//   mod=00 r/m=100 base=101 idx + disp32
//   mod=01 r/m=100 base=101 idx + disp8 + EBP
//   mod=10 r/m=100 base=101 idx + disp32 + EBP
//
//   mod=00 r/m=101 disp32
//
//   idx=100 non index
//
//
// VEX instruction format
//              76543210   76543210   76543210
//             +--------+ +--------+ +--------+
//  3-byte VEX | C4     | |RXBmmmmm| |WvvvvLpp|
//             +--------+ +--------+ +--------+
//
//             +--------+ +--------+
//  2-byte VEX | C3     | |RvvvvLpp|
//             +--------+ +--------+
//
//  R=REX.R inverted form
//  X=REX.X inverted form
//  B=REX.B inverted form
//  W=like REX.W
//  vvvv=register inverted form, 1111 if unused
//  L=0 scalar or 128-bit vector,
//  L=1 256-bit vector
//  pp=00 None
//  pp=01 prefix 66
//  pp=10 prefix F3
//  pp=11 prefix F2
//  mmmmm=00000 reserved
//  mmmmm=00001 implied 0F leading opcode byte
//  mmmmm=00010 implied 0F 38 leading opcode bytes
//  mmmmm=00011 implied 0F 3A leading opcode bytes
//  mmmmm=00100 reserved
//  ...
//  mmmmm=11111 reserved
enum class Mod {
  Disp0 = 0x00,
  Disp8 = 0x40,
  Disp32 = 0x80,
  Reg = 0xC0,
};

enum class Rm {
  Sib = 4,
  Disp32 = 5,
};

enum class Scale {
  None = 1,
  One = 0x00,
  Two = 0x20,
  Four = 0x40,
  RIght = 0xC0,
};

// Rex prefix:
//  Field   Bits    Definition
//  n/a     7:4     0b0100
//  W       3       0=32-bit, 1=64-bit
//  R       2       Extension of the Mod/Rm reg field
//  X       1       Extension of the Mod/Rm SIB index field
//  B       0       Extension of the Mod/Rm r/m, SIB base or Opcode reg field
enum Rex {
  REX_WRX     = 0x4E,
  REX_WRXB    = 0x4F,
  REX_WRB     = 0x4D,
  REX_WR      = 0x4C,
  REX_WXB     = 0x4B,
  REX_WX      = 0x4A,
  REX_WB      = 0x49,
  REX_W       = 0x48,
  REX_RXB     = 0x47,
  REX_RX      = 0x46,
  REX_RB      = 0x45,
  REX_R       = 0x44,
  REX_X       = 0x42,
  REX_XB      = 0x43,
  REX_B       = 0x41,
  REX         = 0x40,
};

enum class Tttn {
  Overflow = 0,
  NoOverflow = 1,
  Below = 2,
  NotBelow = 3,
  Equal = 4,
  NotEqual = 5,
  BelowOrEqual = 6,
  Above = 7,
  Sign = 8,
  NotSign = 9,
  Parity = 10,
  NotParity = 11,
  LessThan = 12,
  GreaterOrEqual = 13,
  LessThanOrEqual = 14,
  GreaterThan = 15,
};

}  // namespace isa
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ISA_X64_H_
