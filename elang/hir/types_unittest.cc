// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "gtest/gtest.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// TypesTest offers HIR factories.
//
class TypesTest : public ::testing::Test {
 protected:
  TypesTest();

  Factory* factory() { return factory_.get(); }
  TypeFactory* types() { return factory_->types(); }
  Zone* zone() { return factory_->zone(); }

 private:
  std::unique_ptr<Factory> factory_;
};

TypesTest::TypesTest() : factory_(new Factory()) {
}

TEST_F(TypesTest, BoolType) {
  auto const bool_type = types()->GetBoolType();
  auto const true_value = bool_type->NewLiteral(true);
  auto const false_value = bool_type->NewLiteral(false);
  EXPECT_NE(false_value, true_value);
  EXPECT_EQ(false_value, bool_type->zero());
}

TEST_F(TypesTest, FunctionType) {
  auto const return_type = types()->GetInt32Type();
  auto const void_type = types()->GetVoidType();
  auto const function_type = types()->NewFunctionType(return_type, void_type);
  EXPECT_EQ(return_type, function_type->return_type());
  EXPECT_EQ(void_type, function_type->parameters_type());
  auto const other = types()->NewFunctionType(return_type, void_type);
  EXPECT_EQ(function_type, other);
}

}  // namespace hir
}  // namespace elang
