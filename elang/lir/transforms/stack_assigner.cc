// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/stack_assigner.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// StackAssigner
//
StackAssigner::StackAssigner(RegisterAssignments* register_assignments,
                             const StackAssignments& assignments)
    : assignments_(assignments), register_assignments_(register_assignments) {
}

StackAssigner::~StackAssigner() {
}

}  // namespace lir
}  // namespace elang
