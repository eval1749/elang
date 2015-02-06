// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_visitor.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

#define V(Name, ...) \
  void Name::Accept(LiteralVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_LIR_LITERAL(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Literal
//
Literal::Literal() {
}

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
BasicBlock::BasicBlock(Value value)
    : function_(nullptr), id_(0), value_(value) {
}

Instruction* BasicBlock::first_instruction() const {
  return instructions_.first_node();
}

Instruction* BasicBlock::last_instruction() const {
  return instructions_.last_node();
}

PhiInstructionList BasicBlock::phi_instructions() const {
  return PhiInstructionList(phi_instructions_);
}

BasicBlockSuccessors BasicBlock::successors() const {
  return BasicBlockSuccessors(this);
}

// BasicBlockSuccessors
BasicBlockSuccessors::BasicBlockSuccessors(const BasicBlock* basic_block)
    : basic_block_(basic_block) {
}

BasicBlockSuccessors::Iterator BasicBlockSuccessors::begin() const {
  auto inputs = basic_block_->last_instruction()->inputs();
  auto it = inputs.begin();
  while (it != inputs.end() && !(*it).is_literal())
    ++it;
  return Iterator(it);
}

BasicBlockSuccessors::Iterator BasicBlockSuccessors::end() const {
  return Iterator(basic_block_->last_instruction()->inputs().end());
}

BasicBlockSuccessors::Iterator::Iterator(const OperandIterator& iterator)
    : IteratorOnIterator(iterator) {
}

BasicBlock* BasicBlockSuccessors::Iterator::operator*() const {
  return (*iterator())->as<BasicBlock>();
}

//////////////////////////////////////////////////////////////////////
//
// Function
//
Function::Function(Value value) : value_(value) {
}

BasicBlock* Function::entry_block() const {
  auto const block = basic_blocks_.first_node();
  DCHECK(block->first_instruction()->is<EntryInstruction>());
  return block;
}

BasicBlock* Function::exit_block() const {
  auto const block = basic_blocks_.last_node();
  DCHECK(block->first_instruction()->is<ExitInstruction>());
  return block;
}

int Function::id() const {
  return value_.data;
}

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
  DCHECK_LT(current_, instruction_->CountInputs());
  ++current_;
  return *this;
}

Value OperandIterator::operator*() const {
  DCHECK_LT(current_, instruction_->CountInputs());
  return instruction_->input(current_);
}

Value OperandIterator::operator->() const {
  DCHECK_LT(current_, instruction_->CountInputs());
  return instruction_->input(current_);
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
// Simple Literals
//
#define V(Name, name, c_type, ...) \
  Name##Literal::Name##Literal(c_type data) : data_(data) {}
FOR_EACH_LIR_SIMPLE_LITERAL(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// StringLiteral
//
StringLiteral::StringLiteral(base::StringPiece16 data) : data_(data) {
}

}  // namespace lir
}  // namespace elang
