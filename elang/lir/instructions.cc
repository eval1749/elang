// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/value.h"

#ifdef ELANG_TARGET_ARCH_X64
#include "elang/lir/instructions_x64.h"
#endif

namespace elang {
namespace lir {

// Constructors are implemented in "instructions_${arch}.cc".

#define V(Name, ...)                                            \
  void Name##Instruction::Accept(InstructionVisitor* visitor) { \
    visitor->Visit##Name(this);                                 \
  }
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

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

Value Instruction::input(int index) const {
  DCHECK_GE(index, 0);
  DCHECK_LE(index, CountInputs());
  return InputValues()[index];
}

Instruction::Values Instruction::inputs() const {
  auto const start = InputValues();
  return Values(start, start + CountInputs());
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

bool ExitInstruction::IsTerminator() const {
  return true;
}

bool JumpInstruction::IsTerminator() const {
  return true;
}

bool RetInstruction::IsTerminator() const {
  return true;
}

}  // namespace lir
}  // namespace elang
