// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/instruction_factory.h"

#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/instruction_visitor.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace hir {

#define V(Name, ...)                                                     \
  void InstructionVisitor::Visit##Name(Name##Instruction* instruction) { \
    DoDefaultVisit(instruction);                                         \
  }                                                                      \
  void Name##Instruction::Accept(InstructionVisitor* visitor) {          \
    visitor->Visit##Name(this);                                          \
  }
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

void InstructionVisitor::DoDefaultVisit(Instruction* instruction) {
  DCHECK(instruction);
}

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
  instr->InitInputAt(0, condition);
  instr->InitInputAt(1, true_block);
  instr->InitInputAt(2, false_block);
  return instr;
}

Instruction* InstructionFactory::NewBranchInstruction(
    BasicBlock* target_block) {
  auto const instr = new (zone()) JumpInstruction(void_type());
  instr->InitInputAt(0, target_block);
  return instr;
}

Instruction* InstructionFactory::NewCallInstruction(Value* callee,
                                                    Value* arguments) {
  auto const callee_type = callee->type()->as<FunctionType>();
  DCHECK(callee_type);
  auto const instr = new (zone()) CallInstruction(callee_type->return_type());
  instr->InitInputAt(0, callee);
  instr->InitInputAt(1, arguments);
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
  instr->InitInputAt(0, pointer);
  return instr;
}

PhiInstruction* InstructionFactory::NewPhiInstruction(Type* output_type) {
  return new (zone()) PhiInstruction(output_type);
}

Instruction* InstructionFactory::NewReturnInstruction(Value* value,
                                                      BasicBlock* exit_block) {
  auto const instr = new (zone()) ReturnInstruction(void_type());
  instr->InitInputAt(0, value);
  instr->InitInputAt(1, exit_block);
  return instr;
}

Instruction* InstructionFactory::NewStoreInstruction(Value* pointer,
                                                     Value* value) {
  DCHECK(pointer->type()->is<PointerType>());
  DCHECK(!value->type()->is<VoidType>());
  auto const instr = new (zone()) StoreInstruction(void_type());
  instr->InitInputAt(0, pointer);
  instr->InitInputAt(1, value);
  return instr;
}

}  // namespace hir
}  // namespace elang
