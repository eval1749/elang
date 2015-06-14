// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_TARGETS_REGISTER_X64_H_
#define ELANG_TARGETS_REGISTER_X64_H_

#include <iosfwd>

namespace elang {
namespace targets {
namespace x64 {

enum class OperandSize {
  Is0,
  Is8,
  Is16,
  Is32,
  Is64,
  Is128,
  Is256,
};

#define FOR_EACH_X64_REGISTER(V) \
  V(None, 0)                     \
  /* 64-bit register */          \
  V(RAX, 0x0400)                 \
  V(RCX, 0x0401)                 \
  V(RDX, 0x0402)                 \
  V(RBX, 0x0403)                 \
  V(RSP, 0x0404)                 \
  V(RBP, 0x0405)                 \
  V(RSI, 0x0406)                 \
  V(RDI, 0x0407)                 \
  V(R8, 0x0408)                  \
  V(R9, 0x0409)                  \
  V(R10, 0x040A)                 \
  V(R11, 0x040B)                 \
  V(R12, 0x040C)                 \
  V(R13, 0x040D)                 \
  V(R14, 0x040E)                 \
  V(R15, 0x040F)                 \
  V(RIP, 0x0410)                 \
  /* 32-bit register */          \
  V(EAX, 0x0300)                 \
  V(ECX, 0x0301)                 \
  V(EDX, 0x0302)                 \
  V(EBX, 0x0303)                 \
  V(ESP, 0x0304)                 \
  V(EBP, 0x0305)                 \
  V(ESI, 0x0306)                 \
  V(EDI, 0x0307)                 \
  V(R8D, 0x0308)                 \
  V(R9D, 0x0309)                 \
  V(R10D, 0x030A)                \
  V(R11D, 0x030B)                \
  V(R12D, 0x030C)                \
  V(R13D, 0x030D)                \
  V(R14D, 0x030E)                \
  V(R15D, 0x030F)                \
  /* 16-bit register */          \
  V(AX, 0x0200)                  \
  V(CX, 0x0201)                  \
  V(DX, 0x0202)                  \
  V(BX, 0x0203)                  \
  V(SP, 0x0204)                  \
  V(BP, 0x0205)                  \
  V(SI, 0x0206)                  \
  V(DI, 0x0207)                  \
  V(R8W, 0x0208)                 \
  V(R9W, 0x0209)                 \
  V(R10W, 0x020A)                \
  V(R11W, 0x020B)                \
  V(R12W, 0x020C)                \
  V(R13W, 0x020D)                \
  V(R14W, 0x020E)                \
  V(R15W, 0x020F)                \
  /* 8-bit register */           \
  V(AL, 0x0100)                  \
  V(CL, 0x0101)                  \
  V(DL, 0x0102)                  \
  V(BL, 0x0103)                  \
  V(SPL, 0x0104)                 \
  V(BPL, 0x0105)                 \
  V(SIL, 0x0106)                 \
  V(DIL, 0x0107)                 \
  V(R8B, 0x0108)                 \
  V(R9B, 0x0109)                 \
  V(R10B, 0x010A)                \
  V(R11B, 0x010B)                \
  V(R12B, 0x010C)                \
  V(R13B, 0x010D)                \
  V(R14B, 0x010E)                \
  V(R15B, 0x010F)                \
  V(AH, 0x0110)                  \
  V(CH, 0x0111)                  \
  V(DH, 0x0112)                  \
  V(BH, 0x0113)                  \
  /* 128-bit registers */        \
  V(XMM0, 0x1000)                \
  V(XMM1, 0x1001)                \
  V(XMM2, 0x1002)                \
  V(XMM3, 0x1003)                \
  V(XMM4, 0x1004)                \
  V(XMM5, 0x1005)                \
  V(XMM6, 0x1006)                \
  V(XMM7, 0x1007)                \
  V(XMM8, 0x1008)                \
  V(XMM9, 0x1009)                \
  V(XMM10, 0x100A)               \
  V(XMM11, 0x100B)               \
  V(XMM12, 0x100C)               \
  V(XMM13, 0x100D)               \
  V(XMM14, 0x100E)               \
  V(XMM15, 0x100F)               \
  /* 256-bit registers */        \
  V(YMM0, 0x1100)                \
  V(YMM1, 0x1101)                \
  V(YMM2, 0x1102)                \
  V(YMM3, 0x1103)                \
  V(YMM4, 0x1104)                \
  V(YMM5, 0x1105)                \
  V(YMM6, 0x1106)                \
  V(YMM7, 0x1107)                \
  V(YMM8, 0x1108)                \
  V(YMM9, 0x1109)                \
  V(YMM10, 0x110A)               \
  V(YMM11, 0x110B)               \
  V(YMM12, 0x110C)               \
  V(YMM13, 0x110D)               \
  V(YMM14, 0x110E)               \
  V(YMM15, 0x110F)               \
  /* segment registers */        \
  V(CS, 0x2000)                  \
  V(DS, 0x2001)                  \
  V(ES, 0x2002)                  \
  V(FS, 0x2003)                  \
  V(GS, 0x2004)                  \
  V(SS, 0x2005)

enum class Register {
#define V(name, value) name = value,
  FOR_EACH_X64_REGISTER(V)
#undef V
};

enum class ScaledIndex {
  None,
  Is1,
  Is2,
  Is4,
  Is8,
};

enum class Tttn {
  Overflow = 0,
  NoOverflow = 1,
  Below = 2,
  NotBelow = 3,
  AboveOrEqual = 3,
  Equal = 4,
  NotEqual = 5,
  BelowOrEqual = 6,
  NotAbove = 6,
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

int RegisterKind(Register reg);
Register RegisterOf(OperandSize size, int number);
OperandSize RegisterSizeOf(Register reg);

std::ostream& operator<<(std::ostream& ostream, Register reg);

}  // namespace x64
}  // namespace targets
}  // namespace elang

#endif  // ELANG_TARGETS_REGISTER_X64_H_
