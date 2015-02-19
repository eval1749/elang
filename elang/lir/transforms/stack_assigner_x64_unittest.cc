// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/literals.h"
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/transforms/stack_assigner.h"
#include "elang/lir/transforms/stack_assignments.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirStackAssignerX64Test
//
class LirStackAssignerX64Test : public testing::LirTest {
 protected:
  LirStackAssignerX64Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(LirStackAssignerX64Test);
};

// Test cases...

TEST_F(LirStackAssignerX64Test, Empty) {
  RegisterAssignments register_assignments;
  StackAssignments stack_assignments;
  StackAssigner stack_assigner(factory(), &register_assignments,
                               &stack_assignments);
  stack_assigner.Run();
}

}  // namespace lir
}  // namespace elang
