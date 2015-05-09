// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/testing/analyzer_test.h"

#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/semantics.h"
#include "elang/compiler/source_code_range.h"

namespace elang {
namespace compiler {
namespace sm {

//////////////////////////////////////////////////////////////////////
//
// IrSemanticsTest
//
class IrSemanticsTest : public testing::AnalyzerTest {
 protected:
  IrSemanticsTest();
  ~IrSemanticsTest() override = default;

  Editor* editor() { return &editor_; }
  Factory* factory() { return name_resolver()->factory(); }
  Type* system_int32();
  Type* system_int64();

  Token* NewToken(base::StringPiece name);
  std::string ToString(Semantic* node);

 private:
  sm::Editor editor_;

  DISALLOW_COPY_AND_ASSIGN(IrSemanticsTest);
};

IrSemanticsTest::IrSemanticsTest() : editor_(session()) {
}

Type* IrSemanticsTest::system_int32() {
  return semantics()->SemanticOf(FindClass("System.Int32"))->as<Type>();
}

Type* IrSemanticsTest::system_int64() {
  return semantics()->SemanticOf(FindClass("System.Int64"))->as<Type>();
}

Token* IrSemanticsTest::NewToken(base::StringPiece name) {
  return session()->NewToken(
      SourceCodeRange(),
      TokenData(session()->NewAtomicString(base::UTF8ToUTF16(name))));
}

std::string IrSemanticsTest::ToString(Semantic* semantic) {
  std::stringstream ostream;
  ostream << *semantic;
  return ostream.str();
}

// Test cases...

TEST_F(IrSemanticsTest, ArrayType) {
  auto const type1 = factory()->NewArrayType(system_int32(), {10, 20});
  auto const type2 = factory()->NewArrayType(system_int32(), {10, 20});
  auto const type3 = factory()->NewArrayType(system_int32(), {10});
  EXPECT_EQ(type1, type2)
      << "array type should be unique by element type and dimensions";
  EXPECT_NE(type1, type3);
  EXPECT_EQ(2, type1->rank());
  EXPECT_EQ("System.Int32[10, 20]", ToString(type1));
  EXPECT_EQ("System.Int32[10]", ToString(type3));
}

// Element type of array type is omitting left most rank, e.g.
//  element_type_of(T[A]) = T
//  element_type_of(T[A][B}) = T[B]
//  element_type_of(T[A][B}[C]) = T[B][C]
TEST_F(IrSemanticsTest, ArrayTypeArrayOfArray) {
  auto const type1 = factory()->NewArrayType(system_int32(), {10});
  auto const type2 = factory()->NewArrayType(type1, {20});
  auto const type3 = factory()->NewArrayType(type2, {30});
  EXPECT_EQ("System.Int32[10]", ToString(type1));
  EXPECT_EQ("System.Int32[20][10]", ToString(type2));
  EXPECT_EQ("System.Int32[30][20][10]", ToString(type3));
}

TEST_F(IrSemanticsTest, ArrayTypeUnbound) {
  EXPECT_EQ("System.Int32[]",
            ToString(factory()->NewArrayType(system_int32(), {-1})));
  EXPECT_EQ("System.Int32[,]",
            ToString(factory()->NewArrayType(system_int32(), {-1, -1})));
  EXPECT_EQ("System.Int32[,,]",
            ToString(factory()->NewArrayType(system_int32(), {-1, -1, -1})));
}

TEST_F(IrSemanticsTest, Enum) {
  auto const ns =
      factory()->NewNamespace(factory()->global_namespace(), NewToken("Foo"));
  auto const enum_type =
      factory()->NewEnum(ns, NewToken("Color"), system_int64());
  std::vector<sm::EnumMember*> members{
      factory()->NewEnumMember(enum_type, NewToken("Red")),
      factory()->NewEnumMember(enum_type, NewToken("Green")),
      factory()->NewEnumMember(enum_type, NewToken("Blue"))};
  editor()->FixEnum(enum_type, members);
  EXPECT_EQ("enum Foo.Color : System.Int64 {Red, Green, Blue}",
            ToString(enum_type));
}

TEST_F(IrSemanticsTest, Namespace) {
  auto const ns1 =
      factory()->NewNamespace(factory()->global_namespace(), NewToken("Foo"));
  auto const ns2 = factory()->NewNamespace(ns1, NewToken("Bar"));
  EXPECT_EQ("namespace Foo", ToString(ns1));
  EXPECT_EQ("namespace Foo.Bar", ToString(ns2));
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
