// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_assignments.h"

#include "base/logging.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// StackAssignments
//
StackAssignments::StackAssignments()
    : maximum_arguments_size_(0),
      maximum_variables_size_(0),
      number_of_calls_(0),
      number_of_parameters_(0) {
}

StackAssignments::~StackAssignments() {
}

Value StackAssignments::StackSlotOf(Value proxy) const {
  DCHECK(proxy.is_memory_proxy());
  auto const it = stack_map_.find(proxy);
  DCHECK(it != stack_map_.end());
  return it->second;
}

}  // namespace lir
}  // namespace elang
