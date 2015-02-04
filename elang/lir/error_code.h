// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ERROR_CODE_H_
#define ELANG_LIR_ERROR_CODE_H_

namespace elang {
namespace lir {

#define IGNORE_LIR_ERROR(category, subcategory, name)

#define FOR_EACH_LIR_ERROR_CODE(E, W)   \
  /* Basic Block */                     \
  E(Validate, BasicBlock, Empty)        \
  E(Validate, BasicBlock, Entry)        \
  E(Validate, BasicBlock, Exit)         \
  E(Validate, BasicBlock, NoFunction)   \
  E(Validate, BasicBlock, NoId)         \
  E(Validate, BasicBlock, NoTerminator) \
  /* Function */                        \
  E(Validate, Function, Empty)          \
  E(Validate, Function, NoEntry)        \
  E(Validate, Function, NoExit)         \
  E(Validate, Function, Exit)           \
  /* Instructions */                    \
  E(Validate, Instruction, BasicBlock)  \
  E(Validate, Phi, Count)               \
  E(Validate, Phi, NotFound)

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
