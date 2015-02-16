// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ERROR_CODE_H_
#define ELANG_LIR_ERROR_CODE_H_

namespace elang {
namespace lir {

#define IGNORE_LIR_ERROR(category, subcategory, name)

#define FOR_EACH_LIR_ERROR_CODE(E, W)    \
  /* Basic Block */                      \
  E(Validate, BasicBlock, DeadEnd)       \
  E(Validate, BasicBlock, Empty)         \
  E(Validate, BasicBlock, Entry)         \
  E(Validate, BasicBlock, Exit)          \
  E(Validate, BasicBlock, Function)      \
  E(Validate, BasicBlock, Id)            \
  E(Validate, BasicBlock, Predecessor)   \
  E(Validate, BasicBlock, Successor)     \
  E(Validate, BasicBlock, Terminator)    \
  E(Validate, BasicBlock, Unreachable)   \
  /* Function */                         \
  E(Validate, Function, Empty)           \
  E(Validate, Function, Entry)           \
  E(Validate, Function, Exit)            \
  /* Instructions */                     \
  E(Validate, Instruction, BasicBlock)   \
  E(Validate, Instruction, BlockOperand) \
  E(Validate, Instruction, Entry)        \
  E(Validate, Instruction, Exit)         \
  E(Validate, Instruction, Id)           \
  E(Validate, Instruction, Input)        \
  E(Validate, Instruction, InputSize)    \
  E(Validate, Instruction, InputType)    \
  E(Validate, Instruction, Output)       \
  E(Validate, Instruction, OutputType)   \
  E(Validate, Instruction, Terminator)   \
  E(Validate, PhiInput, Invalid)         \
  E(Validate, PhiInput, Missing)         \
  E(Validate, PhiInput, Multiple)

//////////////////////////////////////////////////////////////////////
//
// ErrorCode
//
enum class ErrorCode {
#define E(category, subcategory, name) category##subcategory##name,
  FOR_EACH_LIR_ERROR_CODE(E, E)
#undef E
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ERROR_CODE_H_
