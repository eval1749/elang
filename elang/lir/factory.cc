// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/factory.h"

#include "elang/base/atomic_string.h"
#include "elang/base/zone.h"
#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_map.h"
#include "elang/lir/target.h"

#ifdef ELANG_TARGET_ARCH_X64
#include "elang/lir/instructions_x64.h"
#endif

namespace elang {
namespace lir {

#define V(Name, ...)                                               \
  void InstructionVisitor::Visit##Name(Name##Instruction* instr) { \
    DoDefaultVisit(instr);                                         \
  }
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

void InstructionVisitor::DoDefaultVisit(Instruction* instr) {
  DCHECK(instr);
}

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory()
    : last_basic_block_id_(0),
      last_condition_id_(0),
      last_instruction_id_(0),
      last_float_register_id_(0),
      last_general_register_id_(0),
      literal_map_(new LiteralMap()) {
}

Factory::~Factory() {
}

Literal* Factory::GetLiteral(Value value) const {
  return literal_map_->GetLiteral(value);
}

BasicBlock* Factory::NewBasicBlock() {
  Value model(Value::Type::Integer, Value::Size::Size8, Value::Kind::Literal);
  auto const block =
      new (zone()) BasicBlock(literal_map_->next_literal_value(model));
  RegisterLiteral(block);
  return block;
}

Value Factory::NewCondition() {
  return Value(Value::Type::Integer, Value::Size::Size8, Value::Kind::Condition,
               ++last_condition_id_);
}

Function* Factory::NewFunction() {
  Value model(Value::Type::Integer, Value::Size::Size8, Value::Kind::Literal);
  auto const function =
      new (zone()) Function(literal_map_->next_literal_value(model));
  RegisterLiteral(function);

  // Since |Editor| uses entry and exit blocks, we can't use editing
  // functions for populating entry and exit block.

  // Make entry and exit block
  //  entry:
  //    entry
  //    ret
  //  exit:
  //    exit
  auto const entry_block = NewBasicBlock();
  function->basic_blocks_.AppendNode(entry_block);
  entry_block->function_ = function;
  entry_block->id_ = NextBasicBlockId();

  auto const exit_block = NewBasicBlock();
  function->basic_blocks_.AppendNode(exit_block);
  exit_block->function_ = function;
  exit_block->id_ = NextBasicBlockId();

  auto const entry_instr = NewEntryInstruction();
  entry_block->instructions_.AppendNode(entry_instr);
  entry_instr->id_ = NextInstructionId();
  entry_instr->basic_block_ = entry_block;

  auto const ret_instr = NewRetInstruction(exit_block);
  entry_block->instructions_.AppendNode(ret_instr);
  ret_instr->id_ = NextInstructionId();
  ret_instr->basic_block_ = entry_block;

  auto const exit_instr = NewExitInstruction();
  exit_block->instructions_.AppendNode(exit_instr);
  exit_instr->id_ = NextInstructionId();
  exit_instr->basic_block_ = exit_block;

  DCHECK(Editor(this, function).Validate());

  return function;
}

Value Factory::NewFloat32Value(float32_t data) {
  auto const it = float32_map_.find(data);
  if (it != float32_map_.end())
    return it->second;
  auto const value = literal_map_->next_literal_value(Value::Float32Literal());
  RegisterLiteral(new (zone()) Float32Literal(data));
  float32_map_[value.data] = value;
  return value;
}

Value Factory::NewFloat64Value(float64_t data) {
  auto const it = float64_map_.find(data);
  if (it != float64_map_.end())
    return it->second;
  auto const value = literal_map_->next_literal_value(Value::Float64Literal());
  RegisterLiteral(new (zone()) Float64Literal(data));
  float64_map_[value.data] = value;
  return value;
}

Value Factory::NewFloatRegister(Value::Size size) {
  return Value::FloatRegister(size, ++last_float_register_id_);
}

Value Factory::NewRegister(Value::Size size) {
  return Value::Register(size, ++last_general_register_id_);
}

Value Factory::NewRegister(Value type) {
  if (type.type == Value::Type::Float)
    return NewFloatRegister(type.size);
  return NewRegister(type.size);
}

Value Factory::NewRegister() {
  return NewRegister(Target::PointerSize());
}

Value Factory::NewIntValue(Value::Size size, int64_t data) {
  if (size == Value::Size::Size8 || size == Value::Size::Size16 ||
      Value::CanBeImmediate(data)) {
    return Value::Immediate(size, data);
  }

  if (size == Value::Size::Size32) {
    auto const it = int32_map_.find(data);
    if (it != int32_map_.end())
      return it->second;
    auto const value = literal_map_->next_literal_value(
        Value(Value::Type::Integer, size, Value::Kind::Literal));
    RegisterLiteral(new (zone()) Int32Literal(data));
    int32_map_[data] = value;
    return value;
  }

  DCHECK(size == Value::Size::Size64);
  auto const it = int64_map_.find(data);
  if (it != int64_map_.end())
    return it->second;
  auto const value = literal_map_->next_literal_value(
      Value(Value::Type::Integer, size, Value::Kind::Literal));
  RegisterLiteral(new (zone()) Int64Literal(data));
  int64_map_[data] = value;
  return value;
}

Value Factory::NewStringValue(base::StringPiece16 data) {
  auto const it = string_map_.find(data);
  if (it != string_map_.end())
    return it->second;
  Value model(Value::Type::Integer, Target::PointerSize(),
              Value::Kind::Literal);
  auto const value = literal_map_->next_literal_value(model);
  auto const literal = new (zone()) StringLiteral(NewString(data));
  RegisterLiteral(literal);
  string_map_[literal->data()] = value;
  return value;
}

Value Factory::NewStringValue(AtomicString* atomic_string) {
  return NewStringValue(atomic_string->string());
}

int Factory::NextBasicBlockId() {
  return ++last_basic_block_id_;
}

int Factory::NextInstructionId() {
  return ++last_instruction_id_;
}

base::StringPiece16 Factory::NewString(base::StringPiece16 string_piece) {
  auto const size = string_piece.size() * sizeof(base::char16);
  auto const data = static_cast<base::char16*>(Allocate(size));
  ::memcpy(data, string_piece.data(), size);
  return base::StringPiece16(data, string_piece.size());
}

void Factory::RegisterLiteral(Literal* literal) {
  literal_map_->RegisterLiteral(literal);
}

// Instructions
#define V(Name, ...)                               \
  Instruction* Factory::New##Name##Instruction() { \
    return new (zone()) Name##Instruction();       \
  }
FOR_EACH_LIR_INSTRUCTION_0_0(V)
#undef V

#define V(Name, ...)                                          \
  Instruction* Factory::New##Name##Instruction(Value input) { \
    return new (zone()) Name##Instruction(input);             \
  }
FOR_EACH_LIR_INSTRUCTION_0_1(V)
#undef V

#define V(Name, ...)                                                        \
  Instruction* Factory::New##Name##Instruction(Value input, Value input2) { \
    return new (zone()) Name##Instruction(input, input2);                   \
  }
FOR_EACH_LIR_INSTRUCTION_0_2(V)
#undef V

#define V(Name, ...)                                                        \
  Instruction* Factory::New##Name##Instruction(Value output, Value input) { \
    return new (zone()) Name##Instruction(output, input);                   \
  }
FOR_EACH_LIR_INSTRUCTION_1_1(V)
#undef V

#define V(Name, ...)                                                     \
  Instruction* Factory::New##Name##Instruction(Value output, Value left, \
                                               Value right) {            \
    return new (zone()) Name##Instruction(output, left, right);          \
  }
FOR_EACH_LIR_INSTRUCTION_1_2(V)
#undef V

Instruction* Factory::NewBranchInstruction(Value condition,
                                           BasicBlock* true_block,
                                           BasicBlock* false_block) {
  DCHECK(false_block->id());
  DCHECK(true_block->id());
  return new (zone()) BranchInstruction(condition, true_block, false_block);
}

Instruction* Factory::NewJumpInstruction(BasicBlock* target_block) {
  DCHECK(target_block->id());
  return new (zone()) JumpInstruction(target_block);
}

Instruction* Factory::NewPCopyInstruction(const std::vector<Value>& outputs,
                                          const std::vector<Value>& inputs) {
  return new (zone()) PCopyInstruction(zone(), outputs, inputs);
}

Instruction* Factory::NewPhiInstruction(Value output) {
  return new (zone()) PhiInstruction(output);
}

Instruction* Factory::NewRetInstruction(BasicBlock* target_block) {
  DCHECK(target_block->id());
  return new (zone()) RetInstruction(target_block);
}

#ifdef ELANG_TARGET_ARCH_X64
Instruction* Factory::NewDivX64Instruction(Value div_output,
                                           Value mod_output,
                                           Value high_left,
                                           Value low_left,
                                           Value right) {
  return new (zone())
      DivX64Instruction(div_output, mod_output, high_left, low_left, right);
}

Instruction* Factory::NewMulX64Instruction(Value high_output,
                                           Value low_output,
                                           Value left,
                                           Value right) {
  return new (zone()) MulX64Instruction(high_output, low_output, left, right);
}
#endif

}  // namespace lir
}  // namespace elang
