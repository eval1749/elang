// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "elang/lir/literal_map.h"

#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LiteralMap
//
LiteralMap::LiteralMap() {
}

LiteralMap::~LiteralMap() {
}

Value LiteralMap::next_literal_value(Value model) const {
  auto const data = static_cast<int>(literals_.size() + 1);
  DCHECK(Value::CanBeImmediate(data));
  model.data = data;
  return model;
}

Instruction* LiteralMap::GetInstruction(Value value) const {
  DCHECK_EQ(Value::Kind::Instruction, value.kind);
  auto const index = static_cast<size_t>(value.data - 1);
  DCHECK_LT(index, instructions_.size());
  return instructions_[index];
}

Literal* LiteralMap::GetLiteral(Value value) const {
  DCHECK_EQ(Value::Kind::Literal, value.kind);
  auto const index = static_cast<size_t>(value.data - 1);
  DCHECK_LT(index, literals_.size());
  return literals_[index];
}

Value LiteralMap::RegisterInstruction(Instruction* instruction) {
  auto const it = instruction_map_.find(instruction);
  if (it != instruction_map_.end())
    return it->second;
  instructions_.push_back(instruction);
  Value value(Value::Type::Integer, ValueSize::Size8, Value::Kind::Instruction,
              static_cast<int>(instructions_.size()));
  instruction_map_[instruction] = value;
  return value;
}

void LiteralMap::RegisterLiteral(Literal* literal) {
  literals_.push_back(literal);
}

}  // namespace lir
}  // namespace elang
