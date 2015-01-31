// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/testing/hir_test.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

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

TEST_F(HirTypesTest, ArrayType) {
  auto const array1 = types()->NewArrayType(int32_type(), {42});
  auto const array2 = types()->NewArrayType(int32_type(), {42});
  EXPECT_EQ(array1, array2);
  EXPECT_EQ("int32[42]", ToString(array1));
}

TEST_F(HirTypesTest, BoolType) {
  auto const type = bool_type()->as<BoolType>();
  auto const true_value = type->NewLiteral(true);
  auto const false_value = type->NewLiteral(false);
  EXPECT_NE(false_value, true_value);
  EXPECT_EQ(false_value, type->zero());
}

TEST_F(HirTypesTest, FunctionType) {
  auto const return_type = int32_type();
  auto const function_type = types()->NewFunctionType(return_type, void_type());
  EXPECT_EQ(return_type, function_type->return_type());
  EXPECT_EQ(void_type(), function_type->parameters_type());
  auto const other = types()->NewFunctionType(return_type, void_type());
  EXPECT_EQ(function_type, other);
}

TEST_F(HirTypesTest, PointerType) {
  auto const pointer1 = types()->NewPointerType(int32_type());
  auto const pointer2 = types()->NewPointerType(int32_type());
  EXPECT_EQ(pointer1, pointer2);
}

TEST_F(HirTypesTest, TupleType) {
  auto const tuple_type = types()->NewTupleType({int32_type(), bool_type()});
  auto const tuple_value = tuple_type->default_value()->as<TupleLiteral>();
  EXPECT_EQ(int32_type(), tuple_type->get(0));
  EXPECT_EQ(bool_type(), tuple_type->get(1));
  EXPECT_EQ(int32_type()->default_value(), tuple_value->get(0));
  EXPECT_EQ(bool_type()->default_value(), tuple_value->get(1));
  EXPECT_EQ("{int32, bool}", ToString(tuple_type));
  EXPECT_EQ("{0, false}", ToString(tuple_value));
}

}  // namespace hir
}  // namespace elang
