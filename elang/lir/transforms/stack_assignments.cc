// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_assignments.h"

#include <algorithm>

#include "base/logging.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// StackAssignments
//
StackAssignments::StackAssignments() : maximum_argc_(0), maximum_size_(0) {
}

StackAssignments::~StackAssignments() {
}

}  // namespace lir
}  // namespace elang
