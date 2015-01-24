// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTION_VISITOR_H_
#define ELANG_HIR_INSTRUCTION_VISITOR_H_

#include "base/macros.h"
#include "elang/hir/instructions_forward.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// InstructionVisitor
//
class InstructionVisitor {
 public:
#define V(Name, ...) virtual void Visit##Name(Name##Instruction* instruction);
  FOR_EACH_HIR_INSTRUCTION(V)
#undef V

 protected:
  InstructionVisitor();
  virtual ~InstructionVisitor();

  virtual void DoDefaultVisit(Instruction* instruction);

 private:
  DISALLOW_COPY_AND_ASSIGN(InstructionVisitor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTION_VISITOR_H_
