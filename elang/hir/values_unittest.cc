// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <sstream>

#include "elang/base/zone.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/formatters/text_formatter.h"
#include "elang/hir/instructions.h"
#include "elang/hir/values.h"
#include "elang/hir/types.h"
#include "gtest/gtest.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// HirValuesTest offers HIR factories.
//
class HirValuesTest : public ::testing::Test {
 protected:
  HirValuesTest();

  Factory* factory() { return factory_.get(); }
  TypeFactory* types() { return factory_->types(); }
  Zone* zone() { return factory_->zone(); }

 private:
  std::unique_ptr<Factory> factory_;
};

HirValuesTest::HirValuesTest() : factory_(new Factory()) {
}

//////////////////////////////////////////////////////////////////////
//
// Function
//
TEST_F(HirValuesTest, Function) {
  auto const void_type = types()->GetVoidType();
  auto const function_type = types()->NewFunctionType(void_type, void_type);
  auto const function = factory()->NewFunction(function_type);
  EXPECT_TRUE(
      function->entry_block()->first_instruction()->is<EntryInstruction>());
  EXPECT_TRUE(
      function->exit_block()->first_instruction()->is<ExitInstruction>());

  std::stringstream stream;
  TextFormatter formatter(&stream);
  formatter.FormatFunction(function);
  EXPECT_EQ(
      "Function void(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  ret void, block2\n"
      "\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      stream.str());
}

}  // namespace hir
}  // namespace elang
