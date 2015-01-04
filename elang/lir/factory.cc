// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "elang/lir/factory.h"

#include "elang/base/zone.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {
namespace isa {
base::StringPiece GetMnemonic(const lir::Instruction* instruction);
}

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory()
    : last_basic_block_id_(0), last_instruction_id_(0), zone_(new Zone()) {
}

Factory::~Factory() {
}

Literal* Factory::GetLiteral(Value value) {
  DCHECK_EQ(Value::Kind::Literal, value.kind);
  auto const index = static_cast<size_t>(value.data - 1);
  DCHECK_LT(index, literals_.size());
  return literals_[index];
}

base::StringPiece Factory::GetMnemonic(const Instruction* instruction) {
  return isa::GetMnemonic(instruction);
}

BasicBlock* Factory::NewBasicBlock() {
  return new (zone()) BasicBlock();
}

Function* Factory::NewFunction() {
  return new (zone()) Function(this);
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

#define V(Name, ...)                                     \
  Name##Instruction* Factory::New##Name##Instruction() { \
    return new (zone()) Name##Instruction(this);         \
  }
FOR_EACH_LIR_INSTRUCTION(V)
#undef V

base::StringPiece16 Factory::NewString(base::StringPiece16 string_piece) {
  auto const size = string_piece.size() * sizeof(base::char16);
  auto const data = static_cast<base::char16*>(zone_->Allocate(size));
  ::memcpy(data, string_piece.data(), size);
  return base::StringPiece16(data, string_piece.size());
}

Value Factory::RegisterLiteral(Literal* literal) {
  literals_.push_back(literal);
  auto const data = static_cast<int>(literals_.size());
  DCHECK(Value::CanBeImmediate(data));
  return Value(Value::Kind::Literal, data);
}

}  // namespace lir
}  // namespace elang
