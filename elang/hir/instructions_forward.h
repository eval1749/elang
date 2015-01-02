// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_FORWARD_H_
#define ELANG_HIR_INSTRUCTIONS_FORWARD_H_

#include <string>

#include "elang/base/types.h"

namespace elang {
namespace hir {

// Forward declarations
class Instruction;

// Visitor |V| takes three parameters:
//  Name         capitalized name for C++ class
//  mnemonic     mnemonic; used for formatting
//  num_operands Number of operands
#define FOR_EACH_HIR_INSTRUCTION(V) \
  V(Call, "call", 2)                \
  V(Entry, "entry", 0)              \
  V(Exit, "exit", 0)                \
  V(Return, "ret", 2)

#define V(Name, ...) class Name##Instruction;
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_FORWARD_H_
