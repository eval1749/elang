// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/testing/hir_test.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// HirTypesTest offers HIR factories.
//
class HirTypesTest : public testing::HirTest {
 protected:
  HirTypesTest() = default;
  ~HirTypesTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(HirTypesTest);
};

TEST_F(HirTypesTest, BoolType) {
  auto const bool_type = types()->GetBoolType();
  auto const true_value = bool_type->NewLiteral(true);
  auto const false_value = bool_type->NewLiteral(false);
  EXPECT_NE(false_value, true_value);
  EXPECT_EQ(false_value, bool_type->zero());
}

TEST_F(HirTypesTest, FunctionType) {
  auto const return_type = types()->GetInt32Type();
  auto const void_type = types()->void_type();
  auto const function_type = types()->NewFunctionType(return_type, void_type);
  EXPECT_EQ(return_type, function_type->return_type());
  EXPECT_EQ(void_type, function_type->parameters_type());
  auto const other = types()->NewFunctionType(return_type, void_type);
  EXPECT_EQ(function_type, other);
}

TEST_F(HirTypesTest, PointerType) {
  auto const pointer1 = types()->NewPointerType(types()->GetInt32Type());
  auto const pointer2 = types()->NewPointerType(types()->GetInt32Type());
  EXPECT_EQ(pointer1, pointer2);
}

}  // namespace hir
}  // namespace elang
