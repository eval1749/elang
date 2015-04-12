// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/testing/optimizer_test.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory_user.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// TypeTest
//
class TypeTest : public testing::OptimizerTest {
 protected:
  TypeTest() = default;
  ~TypeTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TypeTest);
};

TEST_F(TypeTest, ArrayType) {
  auto const type1 = NewArrayType(int32_type(), {42});
  auto const type2 = NewArrayType(int32_type(), {42});
  EXPECT_EQ(type1, type2);
  EXPECT_EQ("int32[42]", ToString(type1));
  EXPECT_FALSE(type1->is_signed());
  EXPECT_TRUE(type1->is_unsigned());
}

TEST_F(TypeTest, BoolType) {
  auto const type1 = bool_type()->as<BoolType>();
  auto const type2 = bool_type()->as<BoolType>();
  EXPECT_EQ(type1, type2);
  EXPECT_EQ("bool", ToString(type1));
  EXPECT_FALSE(bool_type()->is_float());
  EXPECT_TRUE(bool_type()->is_general());
  EXPECT_FALSE(bool_type()->is_integer());
  EXPECT_FALSE(bool_type()->is_numeric());
  EXPECT_FALSE(bool_type()->is_signed());
  EXPECT_TRUE(bool_type()->is_unsigned());
  EXPECT_FALSE(bool_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::General, bool_type()->register_class());
  EXPECT_EQ(1, bool_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, CharType) {
  EXPECT_EQ("char", ToString(char_type()));
  EXPECT_FALSE(char_type()->is_float());
  EXPECT_TRUE(char_type()->is_general());
  EXPECT_FALSE(char_type()->is_integer());
  EXPECT_FALSE(char_type()->is_numeric());
  EXPECT_FALSE(char_type()->is_signed());
  EXPECT_TRUE(char_type()->is_unsigned());
  EXPECT_FALSE(char_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::General, char_type()->register_class());
  EXPECT_EQ(16, char_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, ControlTypeVoid) {
  auto const type = control_type();
  EXPECT_EQ("control", ToString(type));
  EXPECT_FALSE(type->is_float());
  EXPECT_FALSE(type->is_general());
  EXPECT_FALSE(type->is_integer());
  EXPECT_FALSE(type->is_numeric());
  EXPECT_FALSE(type->is_signed());
  EXPECT_TRUE(type->is_unsigned());
  EXPECT_TRUE(type->is_void());
  EXPECT_EQ(Type::RegisterClass::Void, type->register_class());
}

TEST_F(TypeTest, ControlTypeData) {
  auto const type1 = NewControlType(int32_type());
  auto const type2 = NewControlType(int32_type());
  EXPECT_EQ(type1, type2);
  EXPECT_EQ(int32_type(), type1->data_type());
  EXPECT_EQ("control(int32)", ToString(type1));
  EXPECT_FALSE(type1->is_float());
  EXPECT_FALSE(type1->is_general());
  EXPECT_FALSE(type1->is_integer());
  EXPECT_FALSE(type1->is_numeric());
  EXPECT_FALSE(type1->is_signed());
  EXPECT_TRUE(type1->is_unsigned());
  EXPECT_TRUE(type1->is_void());
  EXPECT_EQ(Type::RegisterClass::Void, type1->register_class());
}

TEST_F(TypeTest, EffectType) {
  auto const type = effect_type();
  EXPECT_EQ("effect", ToString(type));
  EXPECT_FALSE(type->is_float());
  EXPECT_FALSE(type->is_general());
  EXPECT_FALSE(type->is_integer());
  EXPECT_FALSE(type->is_numeric());
  EXPECT_FALSE(type->is_signed());
  EXPECT_TRUE(type->is_unsigned());
  EXPECT_TRUE(type->is_void());
  EXPECT_EQ(Type::RegisterClass::Void, type->register_class());
}

TEST_F(TypeTest, ExternalType) {
  auto const type = NewExternalType(NewAtomicString(L"System.Foo"));
  EXPECT_EQ("System.Foo", ToString(type));
  EXPECT_FALSE(type->is_float());
  EXPECT_TRUE(type->is_general());
  EXPECT_FALSE(type->is_integer());
  EXPECT_FALSE(type->is_numeric());
  EXPECT_FALSE(type->is_signed());
  EXPECT_TRUE(type->is_unsigned());
  EXPECT_FALSE(type->is_void());
  EXPECT_EQ(Type::RegisterClass::General, type->register_class());
}

TEST_F(TypeTest, Float32Type) {
  EXPECT_TRUE(float32_type()->is_float());
  EXPECT_FALSE(float32_type()->is_general());
  EXPECT_FALSE(float32_type()->is_integer());
  EXPECT_TRUE(float32_type()->is_numeric());
  EXPECT_TRUE(float32_type()->is_signed());
  EXPECT_FALSE(float32_type()->is_unsigned());
  EXPECT_FALSE(float32_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Float, float32_type()->register_class());
  EXPECT_EQ(32, float32_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, Float64Type) {
  EXPECT_TRUE(float64_type()->is_float());
  EXPECT_FALSE(float64_type()->is_general());
  EXPECT_FALSE(float64_type()->is_integer());
  EXPECT_TRUE(float64_type()->is_numeric());
  EXPECT_TRUE(float64_type()->is_signed());
  EXPECT_FALSE(float64_type()->is_unsigned());
  EXPECT_FALSE(float64_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Float, float64_type()->register_class());
  EXPECT_EQ(64, float64_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, FunctionType) {
  auto const type1 = NewFunctionType(int32_type(), void_type());
  EXPECT_EQ(int32_type(), type1->return_type());
  EXPECT_EQ(void_type(), type1->parameters_type());
  auto const type2 = NewFunctionType(int32_type(), void_type());
  auto const params3 = NewTupleType({float32_type(), float64_type()});
  auto const type3 = NewFunctionType(bool_type(), params3);
  EXPECT_EQ(type1, type2);
  EXPECT_EQ("int32(void)", ToString(type1));
  EXPECT_EQ("bool(float32, float64)", ToString(type3));
  EXPECT_FALSE(type1->is_signed());
  EXPECT_TRUE(type1->is_unsigned());
}

TEST_F(TypeTest, Int16Type) {
  EXPECT_FALSE(int16_type()->is_float());
  EXPECT_FALSE(int16_type()->is_general());
  EXPECT_TRUE(int16_type()->is_integer());
  EXPECT_TRUE(int16_type()->is_numeric());
  EXPECT_TRUE(int16_type()->is_signed());
  EXPECT_FALSE(int16_type()->is_unsigned());
  EXPECT_FALSE(int16_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, int16_type()->register_class());
  EXPECT_EQ(16, int16_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, Int32Type) {
  EXPECT_FALSE(int32_type()->is_float());
  EXPECT_FALSE(int32_type()->is_general());
  EXPECT_TRUE(int32_type()->is_integer());
  EXPECT_TRUE(int32_type()->is_numeric());
  EXPECT_TRUE(int32_type()->is_signed());
  EXPECT_FALSE(int32_type()->is_unsigned());
  EXPECT_FALSE(int32_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, int32_type()->register_class());
  EXPECT_EQ(32, int32_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, Int64Type) {
  EXPECT_FALSE(int64_type()->is_float());
  EXPECT_FALSE(int64_type()->is_general());
  EXPECT_TRUE(int64_type()->is_integer());
  EXPECT_TRUE(int64_type()->is_numeric());
  EXPECT_TRUE(int64_type()->is_signed());
  EXPECT_FALSE(int64_type()->is_unsigned());
  EXPECT_FALSE(int64_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, int64_type()->register_class());
  EXPECT_EQ(64, int64_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, Int8Type) {
  EXPECT_FALSE(int8_type()->is_float());
  EXPECT_FALSE(int8_type()->is_general());
  EXPECT_TRUE(int8_type()->is_integer());
  EXPECT_TRUE(int8_type()->is_numeric());
  EXPECT_TRUE(int8_type()->is_signed());
  EXPECT_FALSE(int8_type()->is_unsigned());
  EXPECT_FALSE(int8_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, int8_type()->register_class());
  EXPECT_EQ(8, int8_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, IntPtrType) {
  EXPECT_FALSE(intptr_type()->is_float());
  EXPECT_FALSE(intptr_type()->is_general());
  EXPECT_TRUE(intptr_type()->is_integer());
  EXPECT_TRUE(intptr_type()->is_numeric());
  EXPECT_TRUE(intptr_type()->is_signed());
  EXPECT_FALSE(intptr_type()->is_unsigned());
  EXPECT_FALSE(intptr_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, intptr_type()->register_class());
  EXPECT_EQ(0, intptr_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, PointerType) {
  auto const type1 = NewPointerType(int32_type());
  auto const type2 = NewPointerType(int32_type());
  EXPECT_EQ(type1, type2);
  EXPECT_EQ("int32*", ToString(type1));
  EXPECT_FALSE(type1->is_float());
  EXPECT_TRUE(type1->is_general());
  EXPECT_FALSE(type1->is_integer());
  EXPECT_FALSE(type1->is_numeric());
  EXPECT_FALSE(type1->is_signed());
  EXPECT_TRUE(type1->is_unsigned());
  EXPECT_FALSE(type1->is_void());
  EXPECT_EQ(Type::RegisterClass::General, type1->register_class());
}

TEST_F(TypeTest, StringType) {
  EXPECT_FALSE(string_type()->is_float());
  EXPECT_TRUE(string_type()->is_general());
  EXPECT_FALSE(string_type()->is_integer());
  EXPECT_FALSE(string_type()->is_numeric());
  EXPECT_FALSE(string_type()->is_signed());
  EXPECT_TRUE(string_type()->is_unsigned());
  EXPECT_FALSE(string_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::General, string_type()->register_class());
}

TEST_F(TypeTest, TupleType) {
  auto const type1 = NewTupleType({int32_type(), bool_type()});
  auto const type2 = NewTupleType({int32_type(), bool_type()});
  EXPECT_EQ(type1, type2);
  EXPECT_EQ(int32_type(), type1->get(0));
  EXPECT_EQ(bool_type(), type1->get(1));
  EXPECT_EQ("(int32, bool)", ToString(type1));
  EXPECT_FALSE(type1->is_signed());
  EXPECT_TRUE(type1->is_unsigned());
}

TEST_F(TypeTest, UInt16Type) {
  EXPECT_FALSE(uint16_type()->is_float());
  EXPECT_FALSE(uint16_type()->is_general());
  EXPECT_TRUE(uint16_type()->is_integer());
  EXPECT_TRUE(uint16_type()->is_numeric());
  EXPECT_FALSE(uint16_type()->is_signed());
  EXPECT_TRUE(uint16_type()->is_unsigned());
  EXPECT_FALSE(uint16_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, int16_type()->register_class());
  EXPECT_EQ(16, uint16_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, UInt32Type) {
  EXPECT_FALSE(uint32_type()->is_float());
  EXPECT_FALSE(uint32_type()->is_general());
  EXPECT_TRUE(uint32_type()->is_integer());
  EXPECT_TRUE(uint32_type()->is_numeric());
  EXPECT_FALSE(uint32_type()->is_signed());
  EXPECT_TRUE(uint32_type()->is_unsigned());
  EXPECT_FALSE(uint32_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, int32_type()->register_class());
  EXPECT_EQ(32, uint32_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, UInt64Type) {
  EXPECT_FALSE(uint64_type()->is_float());
  EXPECT_FALSE(uint64_type()->is_general());
  EXPECT_TRUE(uint64_type()->is_integer());
  EXPECT_TRUE(uint64_type()->is_numeric());
  EXPECT_FALSE(uint64_type()->is_signed());
  EXPECT_TRUE(uint64_type()->is_unsigned());
  EXPECT_FALSE(uint64_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, int64_type()->register_class());
  EXPECT_EQ(64, uint64_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, UInt8Type) {
  EXPECT_FALSE(uint8_type()->is_float());
  EXPECT_FALSE(uint8_type()->is_general());
  EXPECT_TRUE(uint8_type()->is_integer());
  EXPECT_TRUE(uint8_type()->is_numeric());
  EXPECT_FALSE(uint8_type()->is_signed());
  EXPECT_TRUE(uint8_type()->is_unsigned());
  EXPECT_FALSE(uint8_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, int8_type()->register_class());
  EXPECT_EQ(8, uint8_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, UIntPtrType) {
  EXPECT_FALSE(uintptr_type()->is_float());
  EXPECT_FALSE(uintptr_type()->is_general());
  EXPECT_TRUE(uintptr_type()->is_integer());
  EXPECT_TRUE(uintptr_type()->is_numeric());
  EXPECT_FALSE(uintptr_type()->is_signed());
  EXPECT_TRUE(uintptr_type()->is_unsigned());
  EXPECT_FALSE(uintptr_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Integer, intptr_type()->register_class());
  EXPECT_EQ(0, uintptr_type()->as<PrimitiveType>()->bit_size());
}

TEST_F(TypeTest, VoidType) {
  EXPECT_FALSE(void_type()->is_float());
  EXPECT_FALSE(void_type()->is_general());
  EXPECT_FALSE(void_type()->is_integer());
  EXPECT_FALSE(void_type()->is_numeric());
  EXPECT_FALSE(void_type()->is_signed());
  EXPECT_TRUE(void_type()->is_unsigned());
  EXPECT_TRUE(void_type()->is_void());
  EXPECT_EQ(Type::RegisterClass::Void, void_type()->register_class());
  EXPECT_EQ(0, void_type()->as<PrimitiveType>()->bit_size());
}

}  // namespace optimizer
}  // namespace elang
