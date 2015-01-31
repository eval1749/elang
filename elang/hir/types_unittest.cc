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
  auto const type1 = types()->NewArrayType(int32_type(), {42});
  auto const type2 = types()->NewArrayType(int32_type(), {42});
  EXPECT_EQ(type1, type2);
  EXPECT_EQ(type1->default_value(), type1->default_value());
  EXPECT_EQ("int32[42]", ToString(type1));
}

TEST_F(HirTypesTest, BoolType) {
  auto const type1 = bool_type()->as<BoolType>();
  auto const type2 = bool_type()->as<BoolType>();
  EXPECT_EQ(type1, type2);
  EXPECT_EQ(false_value(), type1->default_value());
  EXPECT_EQ("bool", ToString(type1));
}

TEST_F(HirTypesTest, FunctionType) {
  auto const type1 = types()->NewFunctionType(int32_type(), void_type());
  EXPECT_EQ(int32_type(), type1->return_type());
  EXPECT_EQ(void_type(), type1->parameters_type());
  auto const type2 = types()->NewFunctionType(int32_type(), void_type());
  auto const params3 = types()->NewTupleType({float32_type(), float64_type()});
  auto const type3 = types()->NewFunctionType(bool_type(), params3);
  EXPECT_EQ(type1, type2);
  EXPECT_EQ("int32(void)", ToString(type1));
  EXPECT_EQ("bool(float32, float64)", ToString(type3));
}

TEST_F(HirTypesTest, PointerType) {
  auto const type1 = types()->NewPointerType(int32_type());
  auto const type2 = types()->NewPointerType(int32_type());
  EXPECT_EQ(type1, type2);
  EXPECT_EQ(type1->default_value(), type1->default_value());
  EXPECT_EQ("int32*", ToString(type1));
}

TEST_F(HirTypesTest, TupleType) {
  auto const type1 = types()->NewTupleType({int32_type(), bool_type()});
  auto const type2 = types()->NewTupleType({int32_type(), bool_type()});
  EXPECT_EQ(type1, type2);
  EXPECT_EQ(int32_type(), type1->get(0));
  EXPECT_EQ(bool_type(), type1->get(1));
  EXPECT_EQ("{int32, bool}", ToString(type1));
}

}  // namespace hir
}  // namespace elang
