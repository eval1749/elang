// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_OPCODES_X64_H_
#define ELANG_LIR_EMITTERS_OPCODES_X64_H_

#include "elang/targets/instructions_x64.h"

namespace elang {
namespace lir {
namespace isa {

enum class Opcode {
#define V0(opcode, mnemonic) mnemonic = opcode,
#define V1(opcode, mnemonic, format) mnemonic##_##format = opcode,
#define V2(opcode, mnemonic, format1, format2) \
  mnemonic##_##format1##_##format2 = opcode,
#define V3(opcode, mnemonic, format1, format2, format3) \
  mnemonic##_##format1##_##format2##_##format3 = opcode,
  FOR_EACH_X64_OPCODE(V0, V1, V2, V3)
#undef V0
#undef V1
#undef V2
#undef V3

#define VX1(opcode, opext, mnemonic, format) mnemonic##_##format = opcode,
#define VX2(opcode, opext, mnemonic, format1, format2) \
  mnemonic##_##format1##_##format2 = opcode,
      FOR_EACH_X64_OPEXT(VX1, VX2)
#undef VX1
#undef VX2
};

// OpcodeExt is used in r/m field of ModRm.
enum class OpcodeExt {
#define VX1(opcode, opext, mnemonic, format) mnemonic##_##format = opext,
#define VX2(opcode, opext, mnemonic, format1, format2) \
  mnemonic##_##format1##_##format2 = opext,
  FOR_EACH_X64_OPEXT(VX1, VX2)
#undef VX1
#undef VX2
};

}  // namespace isa
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_OPCODES_X64_H_
