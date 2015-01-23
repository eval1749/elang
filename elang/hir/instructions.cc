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

Value* Operands::operator[](int index) const {
  return instruction_->OperandAt(index);
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

#define V(Name, ...)                                      \
  Name##Instruction::Name##Instruction(Type* output_type) \
      : FixedOperandsInstruction(output_type) {}          \
  Opcode Name##Instruction::opcode() const { return Opcode::Name; }
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

void Instruction::Accept(ValueVisitor* visitor) {
  visitor->VisitInstruction(this);
}

// BranchInstruction
bool BranchInstruction::CanBeRemoved() const {
  return false;
}

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
