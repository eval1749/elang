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

}  // namespace isa
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ISA_X64_H_
