// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTION_VISITOR_H_
#define ELANG_LIR_INSTRUCTION_VISITOR_H_

#include "base/logging.h"
#include "elang/lir/instructions_forward.h"
#ifdef ELANG_TARGET_ARCH_X64
#include "elang/lir/instructions_x64_forward.h"
#endif

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// InstructionVisitor
//
class InstructionVisitor {
 public:
#define V(Name, ...) virtual void Visit##Name(Name##Instruction* instruction);
  FOR_EACH_LIR_INSTRUCTION(V)
#undef V

 protected:
  InstructionVisitor();
  virtual ~InstructionVisitor();

  virtual void DoDefaultVisit(Instruction* instruction);

 private:
  DISALLOW_COPY_AND_ASSIGN(InstructionVisitor);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTION_VISITOR_H_
