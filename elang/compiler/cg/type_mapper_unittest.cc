// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/cg_test.h"

#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/cg/type_mapper.h"
#include "elang/compiler/predefined_names.h"
#include "elang/hir/factory.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/types.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// TypeMapperTest
//
class TypeMapperTest : public testing::CgTest {
 protected:
  TypeMapperTest();
  ~TypeMapperTest() override = default;

  TypeMapper* type_mapper() { return &type_mapper_; }

 private:
  TypeMapper type_mapper_;

  DISALLOW_COPY_AND_ASSIGN(TypeMapperTest);
};

TypeMapperTest::TypeMapperTest() : type_mapper_(factory(), name_resolver()) {
}

TEST_F(TypeMapperTest, PrimitiveTypes) {
#define V(Name, ...)                               \
  EXPECT_EQ(factory()->types()->Get##Name##Type(), \
            type_mapper()->Map(                    \
                name_resolver()->GetPredefinedType(PredefinedName::Name)));
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
}

}  // namespace
}  // namespace compiler
}  // namespace elang
