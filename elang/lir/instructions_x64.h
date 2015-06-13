// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_X64_H_
#define ELANG_LIR_INSTRUCTIONS_X64_H_

#include "elang/lir/instructions.h"

namespace elang {
namespace lir {

// DivX64
class ELANG_LIR_EXPORT DivX64Instruction final
    : public InstructionTemplate<2, 3> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(DivX64);

 private:
  DivX64Instruction(Value div_output,
                    Value mod_output,
                    Value high_left,
                    Value low_left,
                    Value right);
};

// SignX64
class ELANG_LIR_EXPORT SignX64Instruction final
    : public InstructionTemplate<1, 1> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(SignX64);

 private:
  SignX64Instruction(Value output, Value input);
};

// UIntDivX64
class ELANG_LIR_EXPORT UIntDivX64Instruction final
    : public InstructionTemplate<2, 3> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(UIntDivX64);

 private:
  UIntDivX64Instruction(Value div_output,
                        Value mod_output,
                        Value high_left,
                        Value low_left,
                        Value right);
};

// UIntMulX64
class ELANG_LIR_EXPORT UIntMulX64Instruction final
    : public InstructionTemplate<2, 2> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(UIntMulX64);

 private:
  UIntMulX64Instruction(Value high_output,
                        Value low_output,
                        Value left,
                        Value right);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_X64_H_
