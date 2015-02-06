// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "base/logging.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/literals.h"
#include "elang/lir/value.h"

#ifdef ELANG_TARGET_ARCH_X64
#include "elang/lir/instructions_x64.h"
#endif

namespace elang {
namespace lir {

base::StringPiece ToStringPiece(Opcode opcode) {
  static const char* const mnemonics[] = {
#define V(Name, mnemonic, ...) mnemonic,
      FOR_EACH_LIR_INSTRUCTION(V)
#undef V
          "Invalid",
  };
  return mnemonics[std::min(static_cast<size_t>(opcode),
                            arraysize(mnemonics) - 1)];
}

// Implement visitor pattern:
//  InstructionVIsitor::VisitXXX(XXXInstruction* instruction)
//  XXXInstruction::Accept(InstructionVisitor* visitor)
#define V(Name, ...)                                               \
  void InstructionVisitor::Visit##Name(Name##Instruction* instr) { \
    DoDefaultVisit(instr);                                         \
  }                                                                \
  void Name##Instruction::Accept(InstructionVisitor* visitor) {    \
    visitor->Visit##Name(this);                                    \
  }
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

// Constructors
#define V(Name, ...) \
  Name##Instruction::Name##Instruction() {}
FOR_EACH_LIR_INSTRUCTION_0_0(V)
#undef V

#define V(Name, ...) \
  Name##Instruction::Name##Instruction(Value input) { InitInput(0, input); }
FOR_EACH_LIR_INSTRUCTION_0_1(V)
#undef V

#define V(Name, ...)                                              \
  Name##Instruction::Name##Instruction(Value left, Value right) { \
    InitInput(0, left);                                           \
    InitInput(1, right);                                          \
  }
FOR_EACH_LIR_INSTRUCTION_0_2(V)
#undef V

#define V(Name, ...)                                                \
  Name##Instruction::Name##Instruction(Value output, Value input) { \
    DCHECK(output.is_register());                                   \
    InitOutput(0, output);                                          \
    InitInput(0, input);                                            \
  }
FOR_EACH_LIR_INSTRUCTION_1_1(V)
#undef V

#define V(Name, ...)                                             \
  Name##Instruction::Name##Instruction(Value output, Value left, \
                                       Value right) {            \
    DCHECK(output.is_register());                                \
    InitOutput(0, output);                                       \
    InitInput(0, left);                                          \
    InitInput(1, right);                                         \
  }
FOR_EACH_LIR_INSTRUCTION_1_2(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// BasicBlockOperands::Iterator
//
BasicBlockOperands::Iterator::Iterator(const Iterator& other)
    : Iterator(other.pointer_) {
}

BasicBlockOperands::Iterator::Iterator(BasicBlock** pointer)
    : pointer_(pointer) {
}

BasicBlockOperands::Iterator::~Iterator() {
}

BasicBlockOperands::Iterator& BasicBlockOperands::Iterator::operator=(
    const Iterator& other) {
  pointer_ = other.pointer_;
  return *this;
}

BasicBlockOperands::Iterator& BasicBlockOperands::Iterator::operator++() {
  ++pointer_;
  return *this;
}

bool BasicBlockOperands::Iterator::operator==(const Iterator& other) const {
  return pointer_ == other.pointer_;
}

bool BasicBlockOperands::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// BasicBlockOperands
//
BasicBlockOperands::BasicBlockOperands(const BasicBlockOperands& other)
    : BasicBlockOperands(other.start_, other.end_) {
}

BasicBlockOperands::BasicBlockOperands(BasicBlock** start, BasicBlock** end)
    : end_(end), start_(start) {
  DCHECK_LE(start_, end_);
}

BasicBlockOperands::BasicBlockOperands()
    : BasicBlockOperands(nullptr, nullptr) {
}

BasicBlockOperands::~BasicBlockOperands() {
}

BasicBlockOperands& BasicBlockOperands::operator=(
    const BasicBlockOperands& other) {
  end_ = other.end_;
  start_ = other.start_;
  return *this;
}

//////////////////////////////////////////////////////////////////////
//
// Instruction::Values
//
Instruction::Values::Values(const Values& other)
    : end_(other.end_), start_(other.start_) {
}

Instruction::Values::Values(Value* start, Value* end)
    : end_(end), start_(start) {
}

Instruction::Values::~Values() {
}

Instruction::Values& Instruction::Values::operator=(const Values& other) {
  end_ = other.end_;
  start_ = other.start_;
  return *this;
}

//////////////////////////////////////////////////////////////////////
//
// Instruction::Values::Iterator
//
Instruction::Values::Iterator::Iterator(const Iterator& other)
    : pointer_(other.pointer_) {
}

Instruction::Values::Iterator::Iterator(Value* pointer) : pointer_(pointer) {
}

Instruction::Values::Iterator::~Iterator() {
}

Instruction::Values::Iterator& Instruction::Values::Iterator::operator=(
    const Iterator& other) {
  pointer_ = other.pointer_;
  return *this;
}

bool Instruction::Values::Iterator::operator==(const Iterator& other) const {
  return pointer_ == other.pointer_;
}

bool Instruction::Values::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

Instruction::Values::Iterator& Instruction::Values::Iterator::operator++() {
  ++pointer_;
  return *this;
}

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
Instruction::Instruction() : basic_block_(nullptr), id_(0) {
}

BasicBlock* Instruction::block_operand(int index) const {
  auto const operands = block_operands();
  DCHECK_GE(index, 0);
  DCHECK_LT(index, operands.size());
  return operands.start_[index];
}

BasicBlockOperands Instruction::block_operands() const {
  return BasicBlockOperands();
}

Value Instruction::input(int index) const {
  DCHECK_GE(index, 0);
  DCHECK_LE(index, CountInputs());
  return InputValues()[index];
}

Instruction::Values Instruction::inputs() const {
  auto const start = InputValues();
  return Values(start, start + CountInputs());
}

base::StringPiece Instruction::mnemonic() const {
  return ToStringPiece(opcode());
}

Value Instruction::output(int index) const {
  DCHECK_GE(index, 0);
  DCHECK_LE(index, CountOutputs());
  return OutputValues()[index];
}

Instruction::Values Instruction::outputs() const {
  auto const start = OutputValues();
  return Values(start, start + CountOutputs());
}

void Instruction::InitInput(int index, Value new_input) {
  SetInput(index, new_input);
}

void Instruction::InitOutput(int index, Value new_output) {
  SetOutput(index, new_output);
}

void Instruction::SetBlockOperand(int index, BasicBlock* new_value) {
  auto const operands = block_operands();
  DCHECK_GE(index, 0);
  DCHECK_LT(index, operands.size());
  operands.start_[index] = new_value;
}

void Instruction::SetInput(int index, Value new_input) {
  DCHECK_GE(index, 0);
  DCHECK_LE(index, CountInputs());
  InputValues()[index] = new_input;
}

void Instruction::SetOutput(int index, Value new_output) {
  DCHECK_GE(index, 0);
  DCHECK_LE(index, CountOutputs());
  DCHECK(new_output.is_register());
  OutputValues()[index] = new_output;
}

bool Instruction::IsTerminator() const {
  return false;
}

// BranchInstruction
BranchInstruction::BranchInstruction(Value condition,
                                     BasicBlock* true_block,
                                     BasicBlock* false_block) {
  InitInput(0, condition);
  InitBasicBlock(0, true_block);
  InitBasicBlock(1, false_block);
}

// JumpInstruction
JumpInstruction::JumpInstruction(BasicBlock* target_block) {
  InitBasicBlock(0, target_block);
}

// PCopyInstruction
PCopyInstruction::PCopyInstruction(Zone* zone,
                                   const std::vector<Value>& outputs,
                                   const std::vector<Value>& inputs)
    : inputs_(zone, inputs), outputs_(zone, outputs) {
  DCHECK_EQ(inputs_.size(), outputs_.size());
#ifndef NDEBUG
  for (auto const output : outputs_)
    DCHECK(!output.is_read_only());
#endif
}

// PCopyInstruction Instruction operand protocol
int PCopyInstruction::CountInputs() const {
  return static_cast<int>(inputs_.size());
}

int PCopyInstruction::CountOutputs() const {
  return static_cast<int>(outputs_.size());
}

Value* PCopyInstruction::InputValues() const {
  return const_cast<PCopyInstruction*>(this)->inputs_.data();
}

Value* PCopyInstruction::OutputValues() const {
  return const_cast<PCopyInstruction*>(this)->outputs_.data();
}

// PhiInput
PhiInput::PhiInput(BasicBlock* basic_block, Value value)
    : basic_block_(basic_block), value_(value) {
}

// PhiInstruction
PhiInstruction::PhiInstruction(Value output) : output_(output) {
}

Value PhiInstruction::input_of(BasicBlock* basic_block) const {
  auto const phi_input = FindPhiInputFor(basic_block);
  DCHECK(phi_input);
  return phi_input->value();
}

int PhiInstruction::CountInputs() const {
  NOTREACHED();
  return 0;
}

int PhiInstruction::CountOutputs() const {
  return 1;
}

PhiInput* PhiInstruction::FindPhiInputFor(BasicBlock* block) const {
  for (auto phi_input : phi_inputs_) {
    if (phi_input->basic_block() == block)
      return phi_input;
  }
  return nullptr;
}

Value* PhiInstruction::InputValues() const {
  NOTREACHED();
  return nullptr;
}

Value* PhiInstruction::OutputValues() const {
  return &const_cast<PhiInstruction*>(this)->output_;
}

// PhiInstructionList
PhiInstructionList::PhiInstructionList(const InstructionList& list)
    : list_(&list) {
}

PhiInstructionList::Iterator PhiInstructionList::begin() const {
  return PhiInstructionList::Iterator(list_->begin());
}

PhiInstructionList::Iterator PhiInstructionList::end() const {
  return PhiInstructionList::Iterator(list_->end());
}

PhiInstructionList::Iterator::Iterator(
    const InstructionList::Iterator& iterator)
    : IteratorOnIterator(iterator) {
}

PhiInstruction* PhiInstructionList::Iterator::operator*() const {
  return (*iterator())->as<PhiInstruction>();
}

// RetInstruction
RetInstruction::RetInstruction() {
}

}  // namespace lir
}  // namespace elang
