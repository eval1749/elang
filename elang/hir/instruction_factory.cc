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

Instruction* InstructionFactory::NewBranchInstruction(Value* condition,
                                                      BasicBlock* true_block,
                                                      BasicBlock* false_block) {
  auto const instr = new (zone()) BranchInstruction(void_type());
  instr->InitOperandAt(0, condition);
  instr->InitOperandAt(1, true_block);
  instr->InitOperandAt(2, false_block);
  return instr;
}

Instruction* InstructionFactory::NewBranchInstruction(
    BasicBlock* target_block) {
  auto const instr = new (zone()) BranchInstruction(void_type());
  instr->InitOperandAt(0, target_block);
  return instr;
}

Instruction* InstructionFactory::NewCallInstruction(Value* callee,
                                                    Value* arguments) {
  auto const callee_type = callee->type()->as<FunctionType>();
  DCHECK(callee_type);
  auto const instr = new (zone()) CallInstruction(callee_type->return_type());
  instr->InitOperandAt(0, callee);
  instr->InitOperandAt(1, arguments);
  return instr;
}

Instruction* InstructionFactory::NewEntryInstruction(Type* output_type) {
  return new (zone()) EntryInstruction(output_type);
}

Instruction* InstructionFactory::NewExitInstruction() {
  return new (zone()) ExitInstruction(void_type());
}

Instruction* InstructionFactory::NewLoadInstruction(Value* pointer) {
  auto const pointer_type = pointer->type()->as<PointerType>();
  DCHECK(pointer_type);
  auto const instr = new (zone()) LoadInstruction(pointer_type->pointee());
  instr->InitOperandAt(0, pointer);
  return instr;
}

PhiInstruction* InstructionFactory::NewPhiInstruction(Type* output_type) {
  return new (zone()) PhiInstruction(output_type);
}

Instruction* InstructionFactory::NewReturnInstruction(Value* value,
                                                      BasicBlock* exit_block) {
  auto const instr = new (zone()) ReturnInstruction(void_type());
  instr->InitOperandAt(0, value);
  instr->InitOperandAt(1, exit_block);
  return instr;
}

Instruction* InstructionFactory::NewStoreInstruction(Value* pointer,
                                                     Value* value) {
  DCHECK(pointer->type()->is<PointerType>());
  DCHECK(!value->type()->is<VoidType>());
  auto const instr = new (zone()) StoreInstruction(void_type());
  instr->InitOperandAt(0, pointer);
  instr->InitOperandAt(1, value);
  return instr;
}

}  // namespace hir
}  // namespace elang
