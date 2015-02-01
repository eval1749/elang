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

Value LiteralMap::next_literal_value() const {
  auto const data = static_cast<int>(literals_.size() + 1);
  DCHECK(Value::CanBeImmediate(data));
  return Value(Value::Kind::Literal, data);
}

Literal* LiteralMap::GetLiteral(Value value) {
  DCHECK_EQ(Value::Kind::Literal, value.kind);
  auto const index = static_cast<size_t>(value.data - 1);
  DCHECK_LT(index, literals_.size());
  return literals_[index];
}

Value LiteralMap::RegisterLiteral(Literal* literal) {
  auto const value = next_literal_value();
  literals_.push_back(literal);
  return value;
}

}  // namespace lir
}  // namespace elang
