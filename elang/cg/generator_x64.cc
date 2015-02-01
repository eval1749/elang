// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/cg/generator.h"

#include "base/logging.h"
#include "elang/hir/instructions.h"
#include "elang/hir/instruction_visitor.h"
#include "elang/hir/values.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace cg {

//////////////////////////////////////////////////////////////////////
//
// Generator
//
lir::Function* Generator::Generate() {
  for (auto const hir_block : hir_function_->basic_blocks()) {
    EditBasicBlock(hir_block);
    for (auto const instruction : hir_block->instructions())
      const_cast<hir::Instruction*>(instruction)->Accept(this);
    editor()->Commit();
  }
  return editor()->function();
}

// hir::InstructionVisitor

// Set return value and emit 'ret' instruction.
void Generator::VisitRet(hir::RetInstruction* instr) {
  DCHECK(instr->input(0)->is<hir::VoidValue>());
  editor()->SetReturn();
}

}  // namespace cg
}  // namespace elang
