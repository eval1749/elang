// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/emitters/code_emitter.h"

#include "elang/lir/emitters/code_buffer.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// CodeEmitter
//
CodeEmitter::CodeEmitter(const Factory* factory,
                         api::MachineCodeBuilder* builder)
    : builder_(builder), factory_(factory) {
}

CodeEmitter::~CodeEmitter() {
}

void CodeEmitter::Process(const Function* function) {
  CodeBuffer code_buffer(function);
  // Generate codes
  {
    auto const handler = NewInstructionHandler(&code_buffer);
    for (auto const block : function->basic_blocks()) {
      code_buffer.StartBasicBlock(block);
      for (auto const instruction : block->instructions())
        instruction->Accept(handler.get());
      code_buffer.EndBasicBlock();
    }
  }
  code_buffer.Finish(factory_, builder_);
}

}  // namespace lir
}  // namespace elang
