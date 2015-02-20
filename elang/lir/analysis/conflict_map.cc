// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/analysis/conflict_map.h"

#include "base/logging.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// ConflictMap
//
ConflictMap::ConflictMap(ConflictMap&& other) : sets_(std::move(other.sets_)) {
}

ConflictMap::ConflictMap() {
}

ConflictMap::~ConflictMap() {
}

ConflictMap& ConflictMap::operator=(ConflictMap&& other) {
  sets_ = std::move(other.sets_);
  return *this;
}

bool ConflictMap::IsConflict(Value value1, Value value2) const {
  return sets_.InSameSet(value1, value2);
}

}  // namespace lir
}  // namespace elang
