// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/analysis/conflict_map.h"
#include "elang/lir/analysis/conflict_map_builder.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {
namespace {

//////////////////////////////////////////////////////////////////////
//
// ConflictMapTest
//
class ConflictMapTest : public testing::LirTest {
 protected:
  ConflictMapTest() = default;
  ~ConflictMapTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ConflictMapTest);
};

// Test cases...
TEST_F(ConflictMapTest, Basic) {
  auto const function = CreateFunctionEmptySample();
  auto const entry_block = function->entry_block();

  auto const type = Value::Int32Type();

  Editor editor(factory(), function);
  editor.Edit(entry_block);

  auto const reg1 = NewRegister(type);
  editor.Append(NewLiteralInstruction(reg1, Value::SmallInt32(42)));

  auto const reg2 = NewRegister(type);
  editor.Append(NewBitAndInstruction(reg2, reg1, reg1));

  auto const reg3 = NewRegister(type);
  editor.Append(NewBitOrInstruction(reg3, reg1, reg2));

  auto const reg4 = NewRegister(type);
  editor.Append(NewBitOrInstruction(reg4, reg3, reg1));

  ASSERT_EQ("", Commit(&editor));

  auto const conflict_map = ConflictMapBuilder(&editor).Build();
  EXPECT_TRUE(conflict_map.IsConflict(reg1, reg2));
  EXPECT_TRUE(conflict_map.IsConflict(reg1, reg3));

  // input and output aren't conflicted.
  EXPECT_FALSE(conflict_map.IsConflict(reg1, reg4));
  EXPECT_FALSE(conflict_map.IsConflict(reg3, reg4));
}

}  // namespace
}  // namespace lir
}  // namespace elang
