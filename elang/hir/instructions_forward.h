// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_FORWARD_H_
#define ELANG_HIR_INSTRUCTIONS_FORWARD_H_

#include <ostream>
#include <string>

#include "elang/base/float_types.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

// Forward declarations
class Instruction;
enum class Opcode;

// Visitor |V| takes three parameters:
//  Name         capitalized name for C++ class
//  mnemonic     mnemonic; used for formatting
//  num_values Number of values
#define FOR_EACH_HIR_INSTRUCTION(V) \
  V(Branch, "branch", 3)            \
  V(Call, "call", 2)                \
  V(Entry, "entry", 0)              \
  V(Exit, "exit", 0)                \
  V(Jump, "jump", 1)                \
  V(Return, "ret", 2)

#define V(Name, ...) class Name##Instruction;
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction& instruction);

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream, Opcode opcode);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_FORWARD_H_
