// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/instruction_visitor.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// InstructionVisitor
//
InstructionVisitor::InstructionVisitor() {
}

InstructionVisitor::~InstructionVisitor() {
}

void InstructionVisitor::DoDefaultVisit(Instruction* instr) {
  DCHECK(instr);
}

}  // namespace lir
}  // namespace elang
