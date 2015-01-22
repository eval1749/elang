// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/value_visitor.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Operands
//
Operands::Operands(const Instruction* instruction) : instruction_(instruction) {
}

Operands::~Operands() {
}

Operands& Operands::operator=(const Operands& other) {
  DCHECK_EQ(instruction_, other.instruction_);
  instruction_ = other.instruction_;
  return *this;
}

Operands::Iterator Operands::begin() {
  return Iterator(instruction_, 0);
}

Operands::Iterator Operands::end() {
  return Iterator(instruction_, instruction_->CountOperands());
}

Operands::Iterator::Iterator(const Instruction* instruction, int current)
    : current_(current), instruction_(instruction) {
}

Operands::Iterator::Iterator(const Iterator& other)
    : current_(other.current_), instruction_(other.instruction_) {
}

Operands::Iterator::~Iterator() {
}

Operands::Iterator& Operands::Iterator::operator=(const Iterator& other) {
  DCHECK_EQ(instruction_, other.instruction_);
  instruction_ = other.instruction_;
  return *this;
}

Operands::Iterator& Operands::Iterator::operator++() {
  DCHECK_LT(current_, instruction_->CountOperands());
  ++current_;
  return *this;
}

Value* Operands::Iterator::operator*() const {
  DCHECK_LT(current_, instruction_->CountOperands());
  return instruction_->OperandAt(current_);
}

Value* Operands::Iterator::operator->() const {
  DCHECK_LT(current_, instruction_->CountOperands());
  return instruction_->OperandAt(current_);
}

bool Operands::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(instruction_, other.instruction_);
  return current_ == other.current_;
}

bool Operands::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
Instruction::Instruction(Type* output_type)
    : Value(output_type), basic_block_(nullptr), id_(0) {
}

Operands Instruction::operands() const {
  return Operands(this);
}

bool Instruction::CanBeRemoved() const {
  return !IsTerminator() && users().empty();
}

bool Instruction::IsTerminator() const {
  return false;
}

#define V(Name, ...) \
  Opcode Name##Instruction::opcode() const { return Opcode::Name; }
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

void Instruction::Accept(ValueVisitor* visitor) {
  visitor->VisitInstruction(this);
}

//////////////////////////////////////////////////////////////////////
//
// BranchInstruction
//
BranchInstruction::BranchInstruction(Type* output_type,
                                     Value* condition,
                                     BasicBlock* then_block,
                                     BasicBlock* else_block)
    : InstructionTemplate(output_type, condition, then_block, else_block) {
  DCHECK(output_type->is<VoidType>());
  DCHECK(condition->type()->is<BoolType>());
}

bool BranchInstruction::CanBeRemoved() const {
  return false;
}

bool BranchInstruction::IsTerminator() const {
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
CallInstruction::CallInstruction(Type* output_type,
                                 Value* callee,
                                 Value* arguments)
    : InstructionTemplate(output_type, callee, arguments) {
}

bool CallInstruction::CanBeRemoved() const {
  // TODO(eval1749) We should return true for known side effect free functions.
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// EntryInstruction
//
EntryInstruction::EntryInstruction(Type* output_type)
    : InstructionTemplate(output_type) {
}

//////////////////////////////////////////////////////////////////////
//
// ExitInstruction
//
ExitInstruction::ExitInstruction(Type* output_type)
    : InstructionTemplate(output_type) {
  DCHECK(output_type->is<VoidType>());
}

bool ExitInstruction::IsTerminator() const {
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
ReturnInstruction::ReturnInstruction(Type* output_type,
                                     Value* value,
                                     BasicBlock* exit_block)
    : InstructionTemplate(output_type, value, exit_block) {
  DCHECK(output_type->is<VoidType>());
  DCHECK(!value->is<BasicBlock>());
  DCHECK(exit_block->last_instruction()->is<ExitInstruction>());
}

bool ReturnInstruction::IsTerminator() const {
  return true;
}

}  // namespace hir
}  // namespace elang
