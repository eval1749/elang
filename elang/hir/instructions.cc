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
#include "elang/hir/value_visitor.h"

namespace elang {
namespace hir {

// boilerplate member functions.
#define V(Name, ...)                                      \
  Name##Instruction::Name##Instruction(Type* output_type) \
      : FixedOperandsInstruction(output_type) {}          \
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

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
Instruction::Instruction(Type* output_type)
    : Value(output_type), basic_block_(nullptr), id_(0) {
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

// BranchInstruction
bool BranchInstruction::CanBeRemoved() const {
  return false;
}

bool BranchInstruction::IsConditional() const {
  return !IsUnconditional();
}

bool BranchInstruction::IsTerminator() const {
  return true;
}

bool BranchInstruction::IsUnconditional() const {
  return operand(0)->is<VoidValue>();
}

bool BranchInstruction::Validate(Editor* editor) const {
  if (!output_type()->is<VoidType>()) {
    editor->Error(ErrorCode::ValidateInstructionOutput, this);
    return false;
  }
  if (!operand(1)->is<BasicBlock>()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 1);
    return false;
  }

  if (IsUnconditional()) {
    if (!operand(2)->is<VoidValue>()) {
      editor->Error(ErrorCode::ValidateInstructionOperand, this, 2);
      return false;
    }
    return true;
  }

  // Conditional branch
  if (!operand(0)->type()->is<BoolType>()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 0);
    return false;
  }
  if (operand(2)->is<BasicBlock>()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 2);
    return false;
  }
  return true;
}

// CallInstruction
bool CallInstruction::CanBeRemoved() const {
  // TODO(eval1749) We should return true for known side effect free functions.
  return false;
}

bool CallInstruction::Validate(Editor* editor) const {
  auto const function_type = operand(0)->type()->as<FunctionType>();
  if (!function_type) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 0);
    return false;
  }
  if (output_type() != function_type->return_type()) {
    editor->Error(ErrorCode::ValidateInstructionOutput, this);
    return false;
  }
  if (operand(1)->type() != function_type->parameters_type()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 1);
    return false;
  }
  return true;
}

// EntryInstruction
bool EntryInstruction::Validate(Editor* editor) const {
  if (output_type() != editor->function()->parameters_type()) {
    editor->Error(ErrorCode::ValidateInstructionOutput, this);
    return false;
  }
  return true;
}

// ExitInstruction
bool ExitInstruction::IsTerminator() const {
  return true;
}

bool ExitInstruction::Validate(Editor* editor) const {
  if (!output_type()->is<VoidType>()) {
    editor->Error(ErrorCode::ValidateInstructionOutput, this);
    return false;
  }
  return true;
}

// LoadInstruction
bool LoadInstruction::Validate(Editor* editor) const {
  auto const pointer_type = operand(0)->type()->as<PointerType>();
  if (!pointer_type) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 0);
    return false;
  }
  if (output_type() != pointer_type->pointee()) {
    editor->Error(ErrorCode::ValidateInstructionOutput, this);
    return false;
  }
  return true;
}

// ReturnInstruction
bool ReturnInstruction::IsTerminator() const {
  return true;
}

bool ReturnInstruction::Validate(Editor* editor) const {
  if (operand(0)->type() != editor->function()->return_type()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 0);
    return false;
  }
  if (operand(1) != editor->function()->exit_block()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 1);
    return false;
  }
  return true;
}

// StoreInstruction
bool StoreInstruction::CanBeRemoved() const {
  return false;
}

bool StoreInstruction::Validate(Editor* editor) const {
  if (!output_type()->is<VoidType>()) {
    editor->Error(ErrorCode::ValidateInstructionOutput, this);
    return false;
  }
  auto const pointer_type = operand(0)->type()->as<PointerType>();
  if (!pointer_type) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 0);
    return false;
  }
  if (operand(1)->type() != pointer_type->pointee()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 1);
    return false;
  }
  return true;
}

}  // namespace hir
}  // namespace elang
