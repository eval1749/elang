// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/testing/analyzer_test.h"

#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics.h"

namespace elang {
namespace compiler {
namespace sm {
namespace {

//////////////////////////////////////////////////////////////////////
//
// IrNodesTest
//
class IrNodesTest : public testing::AnalyzerTest {
 protected:
  IrNodesTest() = default;
  ~IrNodesTest() override = default;

  Factory* factory() { return name_resolver()->factory(); }
  Type* system_int32();
  Type* system_int64();

  std::string ToString(Node* node);

 private:
  DISALLOW_COPY_AND_ASSIGN(IrNodesTest);
};

Type* IrNodesTest::system_int32() {
  return semantics()->ValueOf(FindClass("System.Int32"))->as<Type>();
}

Type* IrNodesTest::system_int64() {
  return semantics()->ValueOf(FindClass("System.Int64"))->as<Type>();
}

std::string IrNodesTest::ToString(Node* node) {
  std::stringstream ostream;
  ostream << *node;
  return ostream.str();
}

// Test cases...

TEST_F(IrNodesTest, ArrayType) {
  auto const type1 = factory()->NewArrayType(system_int32(), {10, 20});
  auto const type2 = factory()->NewArrayType(system_int32(), {10, 20});
  auto const type3 = factory()->NewArrayType(system_int32(), {10});
  EXPECT_EQ(type1, type2)
      << "array type should be unique by element type and dimensions";
  EXPECT_NE(type1, type3);
  EXPECT_EQ(2, type1->rank());
  EXPECT_EQ("System.Int32[10,20]", ToString(type1));
  EXPECT_EQ("System.Int32[10]", ToString(type3));
}

// Element type of array type is omitting left most rank, e.g.
//  element_type_of(T[A]) = T
//  element_type_of(T[A][B}) = T[B]
//  element_type_of(T[A][B}[C]) = T[B][C]
TEST_F(IrNodesTest, ArrayTypeArrayOfArray) {
  auto const type1 = factory()->NewArrayType(system_int32(), {10});
  auto const type2 = factory()->NewArrayType(type1, {20});
  auto const type3 = factory()->NewArrayType(type2, {30});
  EXPECT_EQ("System.Int32[10]", ToString(type1));
  EXPECT_EQ("System.Int32[20][10]", ToString(type2));
  EXPECT_EQ("System.Int32[30][20][10]", ToString(type3));
}

TEST_F(IrNodesTest, ArrayTypeUnbound) {
  EXPECT_EQ("System.Int32[]",
            ToString(factory()->NewArrayType(system_int32(), {-1})));
  EXPECT_EQ("System.Int32[,]",
            ToString(factory()->NewArrayType(system_int32(), {-1, -1})));
  EXPECT_EQ("System.Int32[,,]",
            ToString(factory()->NewArrayType(system_int32(), {-1, -1, -1})));
}

}  // namespace
}  // namespace sm
}  // namespace compiler
}  // namespace elang
