// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "elang/lir/factory.h"

#include "elang/base/zone.h"
#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_map.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory()
    : last_basic_block_id_(0),
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
  auto const block =
      new (zone()) BasicBlock(literal_map_->next_literal_value());
  auto const value = RegisterLiteral(block);
  DCHECK_EQ(block->value(), value);
  return block;
}

Function* Factory::NewFunction() {
  auto const function =
      new (zone()) Function(literal_map_->next_literal_value());
  auto const value = RegisterLiteral(function);
  DCHECK_EQ(function->value(), value);

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

  auto const ret_instr = NewRetInstruction();
  entry_block->instructions_.AppendNode(ret_instr);
  ret_instr->id_ = NextInstructionId();
  ret_instr->basic_block_ = entry_block;

  auto const exit_instr = NewExitInstruction();
  exit_block->instructions_.AppendNode(exit_instr);
  exit_instr->id_ = NextInstructionId();
  exit_instr->basic_block_ = exit_block;

  DCHECK(Editor::Validate(function));

  return function;
}

Value Factory::NewFloat32Value(float32_t data) {
  auto const it = float32_map_.find(data);
  if (it != float32_map_.end())
    return it->second;
  auto const value = RegisterLiteral(new (zone()) Float32Literal(data));
  float32_map_[value.data] = value;
  return value;
}

Value Factory::NewFloat64Value(float64_t data) {
  auto const it = float64_map_.find(data);
  if (it != float64_map_.end())
    return it->second;
  auto const value = RegisterLiteral(new (zone()) Float64Literal(data));
  float64_map_[value.data] = value;
  return value;
}

Value Factory::NewFloatRegister() {
  return Value(Value::Kind::VirtualFloatRegister, ++last_float_register_id_);
}

Value Factory::NewGeneralRegister() {
  return Value(Value::Kind::VirtualGeneralRegister,
               ++last_general_register_id_);
}

Value Factory::NewInt32Value(int32_t data) {
  if (Value::CanBeImmediate(data))
    return Value(Value::Kind::Immediate, data);
  auto const it = int32_map_.find(data);
  if (it != int32_map_.end())
    return it->second;
  auto const value = RegisterLiteral(new (zone()) Int32Literal(data));
  int32_map_[value.data] = value;
  return value;
}

Value Factory::NewInt64Value(int64_t data) {
  if (data >= std::numeric_limits<int32_t>::min() &&
      data <= std::numeric_limits<int32_t>::max()) {
    return NewInt32Value(static_cast<int32_t>(data));
  }
  auto const it = int64_map_.find(data);
  if (it != int64_map_.end())
    return it->second;
  auto const value = RegisterLiteral(new (zone()) Int64Literal(data));
  int64_map_[value.data] = value;
  return value;
}

Value Factory::NewStringValue(base::StringPiece16 data) {
  auto const it = string_map_.find(data);
  if (it != string_map_.end())
    return it->second;
  auto const literal = new (zone()) StringLiteral(NewString(data));
  auto const value = RegisterLiteral(literal);
  string_map_[literal->data()] = value;
  return value;
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

Value Factory::RegisterLiteral(Literal* literal) {
  return literal_map_->RegisterLiteral(literal);
}

// Instructions
#define V(Name, ...)                               \
  Instruction* Factory::New##Name##Instruction() { \
    return new (zone()) Name##Instruction(this);   \
  }
FOR_EACH_LIR_INSTRUCTION_0_0(V)
#undef V

#define V(Name, ...)                                          \
  Instruction* Factory::New##Name##Instruction(Value input) { \
    return new (zone()) Name##Instruction(this, input);       \
  }
FOR_EACH_LIR_INSTRUCTION_0_1(V)
#undef V

#define V(Name, ...)                                                        \
  Instruction* Factory::New##Name##Instruction(Value output, Value input) { \
    return new (zone()) Name##Instruction(this, output, input);             \
  }
FOR_EACH_LIR_INSTRUCTION_1_1(V)
#undef V

Instruction* Factory::NewJumpInstruction(BasicBlock* target_block) {
  DCHECK(target_block->id());
  return new (zone()) JumpInstruction(this, target_block);
}

}  // namespace lir
}  // namespace elang
