// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_INSTRUCTION_HANDLER_H_
#define ELANG_LIR_EMITTERS_INSTRUCTION_HANDLER_H_

#include "elang/lir/instruction_visitor.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// InstructionHandler
//
class InstructionHandler {
 public:
  virtual ~InstructionHandler();

  virtual void Handle(Instruction* instr) = 0;

 protected:
  InstructionHandler();

 private:
  DISALLOW_COPY_AND_ASSIGN(InstructionHandler);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_INSTRUCTION_HANDLER_H_
