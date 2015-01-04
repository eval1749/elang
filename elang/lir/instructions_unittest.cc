// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LirInstructionTest offers HIR factories.
//
class LirInstructionTest : public ::testing::Test {
 protected:
  LirInstructionTest();

  Factory* factory() { return factory_.get(); }

 private:
  std::unique_ptr<Factory> factory_;
};

LirInstructionTest::LirInstructionTest() : factory_(new Factory()) {
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
TEST_F(LirInstructionTest, CallInstruction) {
  auto const instr = factory()->NewCallInstruction();
  EXPECT_TRUE(instr->is<CallInstruction>());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(0, instr->id());
}

}  // namespace lir
}  // namespace elang
