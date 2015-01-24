// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/editor.h"
#include "elang/hir/error_code.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"
#include "elang/hir/value_visitor.h"

namespace elang {
namespace hir {

// boilerplate member functions.
#define V(Name, mnemonic, Super, ...)                     \
  Name##Instruction::Name##Instruction(Type* output_type) \
      : Super##OperandsInstruction(output_type) {}        \
  Opcode Name##Instruction::opcode() const { return Opcode::Name; }
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// OperandIterator
//
OperandIterator::OperandIterator(const Instruction* instruction, int current)
    : current_(current), instruction_(instruction) {
}

OperandIterator::OperandIterator(const OperandIterator& other)
    : current_(other.current_), instruction_(other.instruction_) {
}

OperandIterator::~OperandIterator() {
}

OperandIterator& OperandIterator::operator=(const OperandIterator& other) {
  DCHECK_EQ(instruction_, other.instruction_);
  instruction_ = other.instruction_;
  return *this;
}

OperandIterator& OperandIterator::operator++() {
  DCHECK_LT(current_, instruction_->CountOperands());
  ++current_;
  return *this;
}

Value* OperandIterator::operator*() const {
  DCHECK_LT(current_, instruction_->CountOperands());
  return instruction_->operand(current_);
}

Value* OperandIterator::operator->() const {
  DCHECK_LT(current_, instruction_->CountOperands());
  return instruction_->operand(current_);
}

bool OperandIterator::operator==(const OperandIterator& other) const {
  DCHECK_EQ(instruction_, other.instruction_);
  return current_ == other.current_;
}

bool OperandIterator::operator!=(const OperandIterator& other) const {
  return !operator==(other);
}

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

OperandIterator Operands::begin() {
  return OperandIterator(instruction_, 0);
}

OperandIterator Operands::end() {
  return OperandIterator(instruction_, instruction_->CountOperands());
}

// BranchInstruction
bool BranchInstruction::IsTerminator() const {
  return true;
}

// CallInstruction
bool CallInstruction::CanBeRemoved() const {
  // TODO(eval1749) We should return true for known side effect free functions.
  return false;
}

// ExitInstruction
bool ExitInstruction::IsTerminator() const {
  return true;
}

// Instruction
Instruction::Instruction(Type* output_type)
    : Value(output_type), basic_block_(nullptr), id_(0) {
}

Function* Instruction::function() const {
  return basic_block() ? basic_block()->function() : nullptr;
}

Value* Instruction::operand(int index) const {
  return OperandAt(index);
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

void Instruction::Accept(ValueVisitor* visitor) {
  visitor->VisitInstruction(this);
}

// JumpInstruction
bool JumpInstruction::IsTerminator() const {
  return true;
}

// PhiInput
PhiInput::PhiInput(PhiInstruction* phi, BasicBlock* block, Value* value)
    : basic_block_(block) {
  Init(phi, value);
}

//////////////////////////////////////////////////////////////////////
//
// PhiInstruction
//
Value* PhiInstruction::input_of(BasicBlock* block) const {
  auto const phi_input = FindPhiInputFor(block);
  DCHECK(phi_input);
  return phi_input->value();
}

PhiInput* PhiInstruction::FindPhiInputFor(BasicBlock* block) const {
  for (auto phi_input : phi_inputs_) {
    if (phi_input->basic_block() == block)
      return phi_input;
  }
  return nullptr;
}

// ReturnInstruction
bool ReturnInstruction::IsTerminator() const {
  return true;
}

// StoreInstruction
bool StoreInstruction::CanBeRemoved() const {
  return false;
}

}  // namespace hir
}  // namespace elang
