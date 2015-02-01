// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_FORWARD_H_
#define ELANG_LIR_INSTRUCTIONS_FORWARD_H_

namespace elang {
namespace lir {

// Forward declarations
class Instruction;
class InstructionVisitor;

// Visitor |V| takes three parameters:
//  Name         capitalized name for C++ class
//  mnemonic     mnemonic; used for formatting
#define FOR_EACH_LIR_INSTRUCTION(V) \
  V(Call, call)                     \
  V(Entry, entry)                   \
  V(Exit, exit)                     \
  V(Jump, jump)                     \
  V(Ret, ret)

#define V(Name, ...) class Name##Instruction;
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

namespace isa {
enum class Opcode;
}  //  namespace isa

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_FORWARD_H_
