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
bool BranchInstruction::CanBeRemoved() const {
  return false;
}

int BranchInstruction::CountOperands() const {
  return IsConditionalBranch() ? 3 : 1;
}

bool BranchInstruction::IsConditionalBranch() const {
  return !IsUnconditionalBranch();
}

bool BranchInstruction::IsTerminator() const {
  return true;
}

bool BranchInstruction::IsUnconditionalBranch() const {
  return operand(0)->is<BasicBlock>();
}

bool BranchInstruction::Validate(Editor* editor) const {
  if (!output_type()->is<VoidType>()) {
    editor->Error(ErrorCode::ValidateInstructionOutput, this);
    return false;
  }

  if (IsUnconditionalBranch()) {
    if (!operand(0)->is<BasicBlock>()) {
      editor->Error(ErrorCode::ValidateInstructionOperand, this, 0);
      return false;
    }
    return true;
  }

  DCHECK(IsConditionalBranch());
  if (!operand(0)->type()->is<BoolType>()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 0);
    return false;
  }
  if (!operand(1)->is<BasicBlock>()) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 1);
    return false;
  }
  if (!operand(2)->is<BasicBlock>()) {
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

// Instruction
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

bool Instruction::IsConditionalBranch() const {
  return false;
}

bool Instruction::IsTerminator() const {
  return false;
}

bool Instruction::IsUnconditionalBranch() const {
  return false;
}

void Instruction::Accept(ValueVisitor* visitor) {
  visitor->VisitInstruction(this);
}

// LoadInstruction
bool LoadInstruction::Validate(Editor* editor) const {
  auto const pointer_type = operand(0)->type()->as<PointerType>();
  if (!pointer_type) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 0,
                  operand(0)->type());
    return false;
  }
  if (output_type() != pointer_type->pointee()) {
    editor->Error(ErrorCode::ValidateInstructionOutput, this, pointer_type);
    return false;
  }
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

int PhiInstruction::CountOperands() const {
  NOTREACHED();
  return 0;
}

PhiInput* PhiInstruction::FindPhiInputFor(BasicBlock* block) const {
  for (auto operand : inputs_) {
    if (operand->basic_block() == block)
      return operand;
  }
  return nullptr;
}

bool PhiInstruction::Validate(Editor* editor) const {
  for (auto const predecessor : basic_block()->predecessors()) {
    if (!FindPhiInputFor(predecessor)) {
      editor->Error(ErrorCode::ValidatePhiNotFound, this, predecessor);
      return false;
    }
  }
  // TODO(eval1749) We should check type of `phi` operands are subtype of
  // output type.
  auto position = 0;
  for (auto const operand : inputs_) {
    if (operand->value()->type() != output_type()) {
      editor->Error(ErrorCode::ValidateInstructionOperand, this, position,
                    operand->value());
      return false;
    }
    ++position;
  }
  if (!position) {
    editor->Error(ErrorCode::ValidatePhiCount, this);
    return false;
  }
  if (position == 1)
    editor->Error(ErrorCode::ValidatePhiOne, this);
  return true;
}

// ReturnInstruction
bool ReturnInstruction::IsTerminator() const {
  return true;
}

bool ReturnInstruction::Validate(Editor* editor) const {
  auto const return_type = editor->function()->return_type();
  if (operand(0)->type() != return_type) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this,
                  {editor->NewInt32(0), return_type});
    return false;
  }
  auto const exit_block = editor->function()->exit_block();
  if (operand(1) != exit_block) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this,
                  {editor->NewInt32(1), exit_block});
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
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 0, pointer_type);
    return false;
  }
  // TODO(eval1749) We should check type of operand(2) is subtype of
  // |pointer_type->pointee()|.
  auto const pointee = pointer_type->pointee();
  if (operand(1)->type() != pointee) {
    editor->Error(ErrorCode::ValidateInstructionOperand, this, 1, pointee);
    return false;
  }
  return true;
}

}  // namespace hir
}  // namespace elang
