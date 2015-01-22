// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/instruction_factory.h"

#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace hir {

InstructionFactory::InstructionFactory(Factory* factory,
                                       const FactoryConfig& config)
    : factory_(factory), type_factory_(new TypeFactory(config)) {
}

VoidValue* InstructionFactory::void_value() const {
  return void_type()->zero();
}

VoidType* InstructionFactory::void_type() const {
  return types()->void_type();
}

BranchInstruction* InstructionFactory::NewBranchInstruction(
    Value* condition,
    BasicBlock* true_block,
    BasicBlock* false_block) {
  auto const instr = new (zone()) BranchInstruction(void_type());
  instr->InitOperandAt(0, condition);
  instr->InitOperandAt(1, true_block);
  instr->InitOperandAt(2, false_block);
  return instr;
}

CallInstruction* InstructionFactory::NewCallInstruction(Type* output_type,
                                                        Value* callee,
                                                        Value* arguments) {
  auto const instr = new (zone()) CallInstruction(output_type);
  instr->InitOperandAt(0, callee);
  instr->InitOperandAt(1, arguments);
  return instr;
}

EntryInstruction* InstructionFactory::NewEntryInstruction(Type* output_type) {
  return new (zone()) EntryInstruction(output_type);
}

ExitInstruction* InstructionFactory::NewExitInstruction() {
  return new (zone()) ExitInstruction(void_type());
}

JumpInstruction* InstructionFactory::NewJumpInstruction(
    BasicBlock* target_block) {
  auto const instr = new (zone()) JumpInstruction(void_type());
  instr->InitOperandAt(0, target_block);
  return instr;
}

ReturnInstruction* InstructionFactory::NewReturnInstruction(
    Value* value,
    BasicBlock* exit_block) {
  auto const instr = new (zone()) ReturnInstruction(void_type());
  instr->InitOperandAt(0, value);
  instr->InitOperandAt(1, exit_block);
  return instr;
}

}  // namespace hir
}  // namespace elang
