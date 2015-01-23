// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_ERROR_CODE_H_
#define ELANG_HIR_ERROR_CODE_H_

namespace elang {
namespace hir {

#define IGNORE_HIR_ERROR(category, subcategory, name)

#define FOR_EACH_HIR_ERROR_CODE(E, W)   \
  /* Basic Block */                     \
  E(Validate, BasicBlock, Empty)        \
  E(Validate, BasicBlock, NoFunction)   \
  E(Validate, BasicBlock, NoId)         \
  E(Validate, BasicBlock, NoTerminator) \
  /* Function */                        \
  E(Validate, Function, Empty)          \
  E(Validate, Function, NoEntry)        \
  E(Validate, Function, NoExit)         \
  E(Validate, Function, Exit)           \
  /* Instructions */                    \
  E(Validate, Instruction, NoId)        \
  E(Validate, Instruction, Operand)     \
  E(Validate, Instruction, Terminator)

//////////////////////////////////////////////////////////////////////
//
// ErrorCode
//
enum class ErrorCode {
#define E(category, subcategory, name) category##subcategory##name,
  FOR_EACH_HIR_ERROR_CODE(E, E)
#undef E
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ERROR_CODE_H_
