// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/register_allocation_map.h"

#include "base/logging.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
int RoundUp(int value, int alignment) {
  return (value + alignment - 1) / alignment * alignment;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocator
//
RegisterAllocator::RegisterAllocator() {
}

RegisterAllocator::~RegisterAllocator() {
}

}  // namespace lir
}  // namespace elang
