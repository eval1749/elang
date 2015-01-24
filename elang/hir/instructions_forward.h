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

// Visitor |V| takes three parameters:
//  Name         capitalized name for C++ class
//  mnemonic     mnemonic; used for formatting
// Note: inputs of |PhiInstruction| are stored in |PhiInput|.
#define FOR_EACH_HIR_INSTRUCTION(V) \
  V(Branch, "br", Fixed)            \
  V(Call, "call", Fixed)            \
  V(Entry, "entry", Fixed)          \
  V(Exit, "exit", Fixed)            \
  V(Load, "load", Fixed)            \
  V(Phi, "phi", Fixed)              \
  V(Return, "ret", Fixed)           \
  V(Store, "store", Fixed)

#define V(Name, ...) class Name##Instruction;
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

// Forward declarations
class Instruction;
class InstructionVisitor;
enum class Opcode;
class OperandIterator;
class Operands;
class PhiInput;

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction& instruction);

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream, Opcode opcode);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_FORWARD_H_
