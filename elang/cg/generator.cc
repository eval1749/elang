// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/cg/generator.h"

#include "elang/hir/instructions.h"
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
Generator::Generator(lir::Factory* factory, hir::Function* hir_function)
    : editor_(new lir::Editor(factory, factory->NewFunction())),
      hir_function_(hir_function) {
  block_map_[hir_function->entry_block()] = function()->entry_block();
  block_map_[hir_function->exit_block()] = function()->exit_block();
}

Generator::~Generator() {
}

lir::Factory* Generator::factory() const {
  return editor()->factory();
}

lir::Function* Generator::function() const {
  return editor_->function();
}

void Generator::EditBasicBlock(hir::BasicBlock* hir_block) {
  auto const it = block_map_.find(hir_block);
  if (it != block_map_.end()) {
    auto const block = it->second;
    DCHECK(!block->first_instruction() ||
           block->first_instruction()->is<lir::EntryInstruction>() ||
           block->first_instruction()->is<lir::ExitInstruction>());
    editor()->Edit(block);
    return;
  }
  editor()->EditNewBasicBlock();
  auto const new_lir_block = editor()->basic_block();
  block_map_[hir_block] = new_lir_block;
}

void Generator::Emit(lir::Instruction* instruction) {
  editor()->Append(instruction);
}

void Generator::EmitCopy(lir::Value output, lir::Value input) {
  DCHECK_NE(output, input);
  Emit(factory()->NewCopyInstruction(output, input));
}

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

#define V(Name, ...)                                           \
  void Generator::Visit##Name(hir::Name##Instruction* instr) { \
    auto const output = MapOutput(instr);                      \
    Emit(factory()->New##Name##Instruction(                    \
        output, MapInput(output, instr->input(0)),             \
        MapInput(output, instr->input(1))));                   \
  }
FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
FOR_EACH_BITWISE_BINARY_OPERATION(V)
FOR_EACH_BITWISE_SHIFT_OPERATION(V)
#undef V

}  // namespace cg
}  // namespace elang
