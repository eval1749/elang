// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/testing/analyzer_test.h"

#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/source_code_range.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace sm {

//////////////////////////////////////////////////////////////////////
//
// SemanticTest
//
class SemanticTest : public testing::AnalyzerTest {
 protected:
  SemanticTest();
  ~SemanticTest() override = default;

  Type* bool_type();
  Editor* editor() { return &editor_; }
  Factory* factory() { return editor()->factory(); }
  Type* int32_type();
  Type* int64_type();

  Class* NewClass(Semantic* outer, base::StringPiece name);
  Class* NewClass(base::StringPiece name);
  Value* NewLiteral(Type* type, const TokenData& data);
  Token* NewToken(const TokenData& data);
  Token* NewToken(base::StringPiece name);
  std::string ToString(Semantic* node);

 private:
  sm::Editor editor_;

  DISALLOW_COPY_AND_ASSIGN(SemanticTest);
};

SemanticTest::SemanticTest() : editor_(session()) {
}

Type* SemanticTest::bool_type() {
  return session()->PredefinedTypeOf(PredefinedName::Bool);
}

Type* SemanticTest::int32_type() {
  return session()->PredefinedTypeOf(PredefinedName::Int32);
}

Type* SemanticTest::int64_type() {
  return session()->PredefinedTypeOf(PredefinedName::Int64);
}

Class* SemanticTest::NewClass(Semantic* outer, base::StringPiece name) {
  return factory()->NewClass(outer, Modifiers(), NewToken(name));
}

Class* SemanticTest::NewClass(base::StringPiece name) {
  return NewClass(factory()->global_namespace(), name);
}

Value* SemanticTest::NewLiteral(Type* type, const TokenData& data) {
  return factory()->NewLiteral(type, NewToken(data));
}

Token* SemanticTest::NewToken(const TokenData& data) {
  return session()->NewToken(SourceCodeRange(), data);
}

Token* SemanticTest::NewToken(base::StringPiece name) {
  return NewToken(
      TokenData(session()->NewAtomicString(base::UTF8ToUTF16(name))));
}

std::string SemanticTest::ToString(Semantic* semantic) {
  std::ostringstream ostream;
  ostream << *semantic;
  return ostream.str();
}

TEST_F(SemanticTest, ArrayType) {
  auto const type1 = factory()->NewArrayType(int32_type(), {10, 20});
  auto const type2 = factory()->NewArrayType(int32_type(), {10, 20});
  auto const type3 = factory()->NewArrayType(int32_type(), {10});
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
TEST_F(SemanticTest, ArrayTypeArrayOfArray) {
  auto const type1 = factory()->NewArrayType(int32_type(), {10});
  auto const type2 = factory()->NewArrayType(type1, {20});
  auto const type3 = factory()->NewArrayType(type2, {30});
  EXPECT_EQ("System.Int32[10]", ToString(type1));
  EXPECT_EQ("System.Int32[20][10]", ToString(type2));
  EXPECT_EQ("System.Int32[30][20][10]", ToString(type3));
}

TEST_F(SemanticTest, ArrayTypeUnbound) {
  EXPECT_EQ("System.Int32[]",
            ToString(factory()->NewArrayType(int32_type(), {-1})));
  EXPECT_EQ("System.Int32[,]",
            ToString(factory()->NewArrayType(int32_type(), {-1, -1})));
  EXPECT_EQ("System.Int32[,,]",
            ToString(factory()->NewArrayType(int32_type(), {-1, -1, -1})));
}

TEST_F(SemanticTest, Class) {
  auto const outer =
      factory()->NewNamespace(factory()->global_namespace(), NewToken("Foo"));
  auto const class_type = NewClass(outer, "Bar");
  editor()->FixClassBase(
      class_type,
      {session()->PredefinedTypeOf(PredefinedName::Object)->as<Class>()});
  EXPECT_EQ("Foo.Bar", ToString(class_type));
}

TEST_F(SemanticTest, ClassIntermediate) {
  auto const outer =
      factory()->NewNamespace(factory()->global_namespace(), NewToken("Foo"));
  auto const class_type = NewClass(outer, "Bar");
  EXPECT_EQ("#Foo.Bar", ToString(class_type));
}

TEST_F(SemanticTest, Const) {
  auto const clazz = NewClass("Foo");
  auto const node = factory()->NewConst(clazz, NewToken("Bar"));
  EXPECT_EQ("const ? Foo.Bar = ?", ToString(node));
}

TEST_F(SemanticTest, Enum) {
  auto const outer =
      factory()->NewNamespace(factory()->global_namespace(), NewToken("Foo"));
  auto const enum_base = int64_type();
  auto const enum_type = factory()->NewEnum(outer, NewToken("Color"));
  editor()->FixEnumBase(enum_type, enum_base);
  factory()->NewEnumMember(enum_type, NewToken("Red"));
  factory()->NewEnumMember(enum_type, NewToken("Green"));
  auto const blue = factory()->NewEnumMember(enum_type, NewToken("Blue"));
  editor()->FixEnumMember(
      blue, NewLiteral(enum_base, TokenData(TokenType::Int32Literal, 42)));
  EXPECT_EQ("enum Foo.Color : System.Int64 {Red, Green, Blue = 42}",
            ToString(enum_type));
}

TEST_F(SemanticTest, EnumIntermediate) {
  auto const outer =
      factory()->NewNamespace(factory()->global_namespace(), NewToken("Foo"));
  auto const enum_type = factory()->NewEnum(outer, NewToken("Color"));
  EXPECT_EQ("#enum Foo.Color", ToString(enum_type));
}

TEST_F(SemanticTest, EnumMemberIntermediate) {
  auto const outer =
      factory()->NewNamespace(factory()->global_namespace(), NewToken("Foo"));
  auto const enum_type = factory()->NewEnum(outer, NewToken("Color"));
  auto const enum_member = factory()->NewEnumMember(enum_type, NewToken("Red"));
  EXPECT_EQ("Foo.Color.Red", ToString(enum_member));
}

TEST_F(SemanticTest, Field) {
  auto const class_type = NewClass("Foo");
  auto const field = factory()->NewField(class_type, NewToken("field_"));
  EXPECT_EQ("? Foo.field_", ToString(field));
}

TEST_F(SemanticTest, Method) {
  auto const class_type = NewClass("Foo");
  auto const method_group =
      factory()->NewMethodGroup(class_type, NewToken("Bar"));
  std::vector<Parameter*> parameters{
      factory()->NewParameter(ParameterKind::Required, 0, bool_type(),
                              NewToken("a"), nullptr),
      factory()->NewParameter(ParameterKind::Required, 0, int32_type(),
                              NewToken("b"), nullptr)};
  auto const method_signature =
      factory()->NewSignature(int32_type(), parameters);
  auto const method =
      factory()->NewMethod(method_group, Modifiers(), method_signature);
  EXPECT_EQ("System.Int32 Foo.Bar(System.Bool, System.Int32)",
            ToString(method));
  EXPECT_EQ("(#Foo* this, System.Bool a, System.Int32 b) => System.Int32",
            ToString(method->function_signature()));
}

TEST_F(SemanticTest, Namespace) {
  auto const ns1 =
      factory()->NewNamespace(factory()->global_namespace(), NewToken("Foo"));
  auto const ns2 = factory()->NewNamespace(ns1, NewToken("Bar"));
  EXPECT_EQ("namespace Foo", ToString(ns1));
  EXPECT_EQ("namespace Foo.Bar", ToString(ns2));
}

TEST_F(SemanticTest, PointerType) {
  auto const node = factory()->NewPointerType(int32_type());
  EXPECT_EQ("System.Int32*", ToString(node));
  EXPECT_EQ(node, factory()->NewPointerType(int32_type()));
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
