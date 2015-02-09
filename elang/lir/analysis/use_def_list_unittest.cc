// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/lir/testing/lir_test.h"

#include "elang/lir/analysis/use_def_list.h"
#include "elang/lir/analysis/use_def_list_builder.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {
namespace {

std::string ToString(const UseDefList::Users& users) {
  std::stringstream ostream;
  for (auto const user : users)
    ostream << *user << std::endl;
  return ostream.str();
}

//////////////////////////////////////////////////////////////////////
//
// UseDefListTest
//
class UseDefListTest : public testing::LirTest {
 protected:
  UseDefListTest() = default;
  ~UseDefListTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(UseDefListTest);
};

// Test cases...
TEST_F(UseDefListTest, Basic) {
  auto const function = CreateFunctionEmptySample();
  auto const entry_block = function->entry_block();

  Value type(Value::Type::Integer, ValueSize::Size32);

  Editor editor(factory(), function);
  editor.Edit(entry_block);

  auto const reg1 = NewRegister(type);
  editor.Append(NewLiteralInstruction(reg1, Value::SmallInt32(42)));

  auto const reg2 = NewRegister(type);
  // Even if register is used in same instructions more than once, this
  // instruction is appeared once in use-def list.
  editor.Append(NewBitAndInstruction(reg2, reg1, reg1));

  auto const reg3 = NewRegister(type);
  editor.Append(NewBitOrInstruction(reg3, reg1, reg2));

  ASSERT_EQ("", Commit(&editor));

  auto const use_def_list = UseDefListBuilder(function).Build();
  EXPECT_EQ(
      "bb:15:and %r2 = %r1, %r1\n"
      "bb:16:or %r3 = %r1, %r2\n",
      ToString(use_def_list.UsersOf(reg1)));
  EXPECT_EQ("bb:16:or %r3 = %r1, %r2\n", ToString(use_def_list.UsersOf(reg2)));
  EXPECT_EQ("", ToString(use_def_list.UsersOf(reg3))) << "%r3 isn't used.";
}

}  // namespace
}  // namespace lir
}  // namespace elang
