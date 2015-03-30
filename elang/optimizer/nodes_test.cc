// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "elang/optimizer/testing/optimizer_test.h"

#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// NodeTest
//
class NodeTest : public testing::OptimizerTest {
 protected:
  NodeTest() = default;
  ~NodeTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeTest);
};

TEST_F(NodeTest, BoolNode) {
  EXPECT_EQ(false_value(), false_value());
  EXPECT_EQ(true_value(), true_value());
  EXPECT_EQ("false", ToString(false_value()));
  EXPECT_EQ("true", ToString(true_value()));
}

TEST_F(NodeTest, CallNode) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const callee = NewReference(
      NewFunctionType(void_type(), NewTupleType({int32_type(), int64_type()})),
      NewAtomicString(L"Foo"));
  auto const arguments =
      NewTuple({NewParameter(entry_node, 0), NewParameter(entry_node, 1)});
  auto const node = NewCall(NewEffectGet(entry_node, 1), callee, arguments);
  EXPECT_EQ("(effect, void) %t9 = call(%e3, void(int32, int64) Foo, %t8)",
            ToString(node));
}

TEST_F(NodeTest, CharNode) {
  EXPECT_EQ(NewChar('a'), NewChar('a'));
  EXPECT_NE(NewChar('z'), NewChar('a'));
  EXPECT_EQ("'a'", ToString(NewChar('a')));
  EXPECT_EQ("'\\''", ToString(NewChar('\'')));
}

TEST_F(NodeTest, DynamicCastNode) {
  auto const function = NewSampleFunction(void_type(), int32_type());
  auto const entry_node = function->entry_node();
  auto const node = NewDynamicCast(int64_type(), NewParameter(entry_node, 0));
  EXPECT_EQ("int64 %r7 = dynamic_cast(%r6)", ToString(node));
}

TEST_F(NodeTest, EffectGet) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = NewEffectGet(function->entry_node(), 1);
  EXPECT_EQ("effect %e3 = effect_get(%t1, 1)", ToString(node));
}

TEST_F(NodeTest, EffectPhi) {
  auto const merge_node = NewMerge({});
  auto const node = NewEffectPhi(merge_node);
  EXPECT_EQ("effect %e2 = effect_phi()", ToString(node));
}

TEST_F(NodeTest, EntryNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = function->entry_node();
  EXPECT_EQ("(control, effect, void) %t1 = entry()", ToString(node));
}

TEST_F(NodeTest, ExitNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = function->exit_node();
  EXPECT_EQ("void %r5 = exit(%c4)", ToString(node));
}

TEST_F(NodeTest, Float32Node) {
  EXPECT_EQ(NewFloat32(0), NewFloat32(0));
  EXPECT_NE(NewFloat32(1), NewFloat32(0));
  EXPECT_EQ("0.000000f", ToString(NewFloat32(0.0f)));
  EXPECT_EQ("-1.00000f", ToString(NewFloat32(-1.0f)));
  EXPECT_EQ("3.14000f", ToString(NewFloat32(3.14f)));
}

TEST_F(NodeTest, Float64Node) {
  EXPECT_EQ(NewFloat64(0), NewFloat64(0));
  EXPECT_NE(NewFloat64(1), NewFloat64(0));
  EXPECT_EQ("0.000000", ToString(NewFloat64(0)));
  EXPECT_EQ("-1.00000", ToString(NewFloat64(-1)));
  EXPECT_EQ("3.14000", ToString(NewFloat64(3.14)));
}

#define V(Name, mnemonic)                                                  \
  TEST_F(NodeTest, Float##Name##Node) {                                    \
    auto const function = NewSampleFunction(                               \
        void_type(),                                                       \
        {float32_type(), float32_type(), float64_type(), float64_type()}); \
    auto const entry_node = function->entry_node();                        \
    auto const node32 = NewFloat##Name(NewParameter(entry_node, 0),        \
                                       NewParameter(entry_node, 1));       \
    EXPECT_EQ("float32 %r8 = " mnemonic "(%r7, %r6)", ToString(node32));   \
    auto const node64 = NewFloat##Name(NewParameter(entry_node, 2),        \
                                       NewParameter(entry_node, 3));       \
    EXPECT_EQ("float64 %r11 = " mnemonic "(%r10, %r9)", ToString(node64)); \
  }
V(Add, "fadd")
V(Div, "fdiv")
V(Mod, "fmod")
V(Mul, "fmul")
V(Sub, "fsub")
#undef V

TEST_F(NodeTest, FloatCmpNode) {
  auto const function = NewSampleFunction(
      void_type(),
      {float32_type(), float32_type(), float64_type(), float64_type()});
  auto const entry_node = function->entry_node();
  auto const node32 =
      NewFloatCmp(FloatCondition::OrderedEqual, NewParameter(entry_node, 0),
                  NewParameter(entry_node, 1));
  EXPECT_EQ("bool %r8 = fcmp_eq(%r7, %r6)", ToString(node32));
  auto const node64 =
      NewFloatCmp(FloatCondition::OrderedNotEqual, NewParameter(entry_node, 2),
                  NewParameter(entry_node, 3));
  EXPECT_EQ("bool %r11 = fcmp_ne(%r10, %r9)", ToString(node64));
}

TEST_F(NodeTest, GetNode) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const node = NewGet(entry_node, 2);
  EXPECT_EQ("(int32, int64) %t6 = get(%t1, 2)", ToString(node));
}

TEST_F(NodeTest, Int8Node) {
  EXPECT_EQ(NewInt8(0), NewInt8(0));
  EXPECT_NE(NewInt8(1), NewInt8(0));
  EXPECT_EQ("int8(0)", ToString(NewInt8(0)));
  EXPECT_EQ("int8(-1)", ToString(NewInt8(-1)));
  EXPECT_EQ("int8(127)", ToString(NewInt8(std::numeric_limits<int8_t>::max())));
  EXPECT_EQ("int8(-128)",
            ToString(NewInt8(std::numeric_limits<int8_t>::min())));
}

TEST_F(NodeTest, Int16Node) {
  EXPECT_EQ(NewInt16(0), NewInt16(0));
  EXPECT_NE(NewInt16(1), NewInt16(0));
  EXPECT_EQ("int16(0)", ToString(NewInt16(0)));
  EXPECT_EQ("int16(-1)", ToString(NewInt16(-1)));
  EXPECT_EQ("int16(32767)",
            ToString(NewInt16(std::numeric_limits<int16_t>::max())));
  EXPECT_EQ("int16(-32768)",
            ToString(NewInt16(std::numeric_limits<int16_t>::min())));
}

TEST_F(NodeTest, Int32Node) {
  EXPECT_EQ(NewInt32(0), NewInt32(0));
  EXPECT_NE(NewInt32(1), NewInt32(0));
  EXPECT_EQ("0", ToString(NewInt32(0)));
  EXPECT_EQ("-1", ToString(NewInt32(-1)));
  EXPECT_EQ("2147483647",
            ToString(NewInt32(std::numeric_limits<int32_t>::max())));
  EXPECT_EQ("-2147483648",
            ToString(NewInt32(std::numeric_limits<int32_t>::min())));
}

TEST_F(NodeTest, Int64Node) {
  EXPECT_EQ(NewInt64(0), NewInt64(0));
  EXPECT_NE(NewInt64(1), NewInt64(0));
  EXPECT_EQ("0l", ToString(NewInt64(0)));
  EXPECT_EQ("-1l", ToString(NewInt64(-1)));
  EXPECT_EQ("9223372036854775807l",
            ToString(NewInt64(std::numeric_limits<int64_t>::max())));
  EXPECT_EQ("-9223372036854775808l",
            ToString(NewInt64(std::numeric_limits<int64_t>::min())));
}

#define V(Name, mnemonic)                                                   \
  TEST_F(NodeTest, Int##Name##Node) {                                       \
    auto const function = NewSampleFunction(void_type(), {int32_type(),     \
                                                          int32_type(),     \
                                                          int64_type(),     \
                                                          int64_type(),     \
                                                          uint32_type(),    \
                                                          uint32_type(),    \
                                                          uint64_type(),    \
                                                          uint64_type()});  \
    auto const entry_node = function->entry_node();                         \
    auto const node32 = NewInt##Name(NewParameter(entry_node, 0),           \
                                     NewParameter(entry_node, 1));          \
    EXPECT_EQ("int32 %r8 = " mnemonic "(%r7, %r6)", ToString(node32));      \
    auto const node64 = NewInt##Name(NewParameter(entry_node, 2),           \
                                     NewParameter(entry_node, 3));          \
    EXPECT_EQ("int64 %r11 = " mnemonic "(%r10, %r9)", ToString(node64));    \
    auto const node32u = NewInt##Name(NewParameter(entry_node, 4),          \
                                      NewParameter(entry_node, 5));         \
    EXPECT_EQ("uint32 %r14 = " mnemonic "(%r13, %r12)", ToString(node32u)); \
    auto const node64u = NewInt##Name(NewParameter(entry_node, 6),          \
                                      NewParameter(entry_node, 7));         \
    EXPECT_EQ("uint64 %r17 = " mnemonic "(%r16, %r15)", ToString(node64u)); \
  }
V(Add, "add")
V(BitAnd, "bit_and")
V(BitOr, "bit_or")
V(BitXor, "bit_xor")
V(Div, "div")
V(Mod, "mod")
V(Mul, "mul")
V(Sub, "sub")
#undef V

TEST_F(NodeTest, IntCmpNode) {
  auto const function = NewSampleFunction(
      void_type(), {int32_type(), int32_type(), int64_type(), int64_type()});
  auto const entry_node = function->entry_node();
  auto const node32 =
      NewIntCmp(IntCondition::Equal, NewParameter(entry_node, 0),
                NewParameter(entry_node, 1));
  EXPECT_EQ("bool %r8 = cmp_eq(%r7, %r6)", ToString(node32));
  auto const node64 =
      NewIntCmp(IntCondition::NotEqual, NewParameter(entry_node, 2),
                NewParameter(entry_node, 3));
  EXPECT_EQ("bool %r11 = cmp_ne(%r10, %r9)", ToString(node64));
}

#define V(Name, mnemonic)                                                   \
  TEST_F(NodeTest, Int##Name##Node) {                                       \
    auto const function = NewSampleFunction(                                \
        void_type(),                                                        \
        {int32_type(), int64_type(), uint32_type(), uint64_type()});        \
    auto const entry_node = function->entry_node();                         \
    auto const node32 = NewInt##Name(NewParameter(entry_node, 0),           \
                                     NewParameter(entry_node, 0));          \
    EXPECT_EQ("int32 %r8 = " mnemonic "(%r7, %r6)", ToString(node32));      \
    auto const node64 = NewInt##Name(NewParameter(entry_node, 1),           \
                                     NewParameter(entry_node, 0));          \
    EXPECT_EQ("int64 %r11 = " mnemonic "(%r10, %r9)", ToString(node64));    \
    auto const node32u = NewInt##Name(NewParameter(entry_node, 2),          \
                                      NewParameter(entry_node, 0));         \
    EXPECT_EQ("uint32 %r14 = " mnemonic "(%r13, %r12)", ToString(node32u)); \
    auto const node64u = NewInt##Name(NewParameter(entry_node, 3),          \
                                      NewParameter(entry_node, 0));         \
    EXPECT_EQ("uint64 %r17 = " mnemonic "(%r16, %r15)", ToString(node64u)); \
  }
V(Shl, "shl")
V(Shr, "shr")
#undef V

TEST_F(NodeTest, JumpNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = NewJump(NewGet(function->entry_node(), 0));
  EXPECT_EQ("control %c6 = br(%c2)", ToString(node));
}

TEST_F(NodeTest, ParameterNode) {
  auto const function = NewSampleFunction(void_type(), int32_type());
  auto const entry_node = function->entry_node();
  auto const node = NewParameter(entry_node, 0);
  EXPECT_EQ("int32 %r6 = param(%t1, 0)", ToString(node));
}

TEST_F(NodeTest, ParameterNode2) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const node = NewParameter(entry_node, 1);
  EXPECT_EQ("int64 %r6 = param(%t1, 1)", ToString(node));
}

TEST_F(NodeTest, PhiOperandNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const entry_node = function->entry_node();
  auto const node = NewPhiOperand(NewGet(entry_node, 0), NewBool(false));
  EXPECT_EQ("(control, bool) %t6 = phi_operand(%c2, false)", ToString(node));
}

TEST_F(NodeTest, ReferenceNode) {
  auto const node = NewReference(NewFunctionType(void_type(), int32_type()),
                                 NewAtomicString(L"Foo"));
  EXPECT_EQ("void(int32) Foo", ToString(node));
}

TEST_F(NodeTest, RetNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const entry_node = function->entry_node();
  auto const node =
      NewRet(NewGet(entry_node, 0), NewEffectGet(entry_node, 1), void_value());
  EXPECT_EQ("control %c6 = ret(%c2, %e3, void)", ToString(node));
}

TEST_F(NodeTest, StaticCastNode) {
  auto const function = NewSampleFunction(void_type(), int32_type());
  auto const entry_node = function->entry_node();
  auto const node = NewStaticCast(int64_type(), NewParameter(entry_node, 0));
  EXPECT_EQ("int64 %r7 = static_cast(%r6)", ToString(node));
}

TEST_F(NodeTest, StringNode) {
  EXPECT_EQ("\"abc\"", ToString(NewString(L"abc")));
  EXPECT_EQ("\"123\\n456\"", ToString(NewString(L"123\n456")));
  EXPECT_EQ("\"\\u1234\"", ToString(NewString(L"\u1234")));
}

TEST_F(NodeTest, TupleNode) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const parameters = NewGet(entry_node, 2);
  auto const parameter0 = NewGet(parameters, 0);
  auto const parameter1 = NewGet(parameters, 1);
  auto const node = NewTuple({parameter1, parameter0});
  EXPECT_EQ("(int64, int32) %t9 = tuple(%r8, %r7)", ToString(node));
}

TEST_F(NodeTest, UInt8Node) {
  EXPECT_EQ(NewUInt8(0), NewUInt8(0));
  EXPECT_NE(NewUInt8(1), NewUInt8(0));
  EXPECT_EQ("uint8(0)", ToString(NewUInt8(0)));
  EXPECT_EQ("uint8(255)",
            ToString(NewUInt8(std::numeric_limits<uint8_t>::max())));
  EXPECT_EQ("uint8(0)",
            ToString(NewUInt8(std::numeric_limits<uint8_t>::min())));
}

TEST_F(NodeTest, UInt16Node) {
  EXPECT_EQ(NewUInt16(0), NewUInt16(0));
  EXPECT_NE(NewUInt16(1), NewUInt16(0));
  EXPECT_EQ("uint16(0)", ToString(NewUInt16(0)));
  EXPECT_EQ("uint16(65535)",
            ToString(NewUInt16(std::numeric_limits<uint16_t>::max())));
  EXPECT_EQ("uint16(0)",
            ToString(NewUInt16(std::numeric_limits<uint16_t>::min())));
}

TEST_F(NodeTest, UInt32Node) {
  EXPECT_EQ(NewUInt32(0), NewUInt32(0));
  EXPECT_NE(NewUInt32(1), NewUInt32(0));
  EXPECT_EQ("0u", ToString(NewUInt32(0)));
  EXPECT_EQ("4294967295u",
            ToString(NewUInt32(std::numeric_limits<uint32_t>::max())));
  EXPECT_EQ("0u", ToString(NewUInt32(std::numeric_limits<uint32_t>::min())));
}

TEST_F(NodeTest, UInt64Node) {
  EXPECT_EQ(NewUInt64(0), NewUInt64(0));
  EXPECT_NE(NewUInt64(1), NewUInt64(0));
  EXPECT_EQ("0ul", ToString(NewUInt64(0)));
  EXPECT_EQ("18446744073709551615ul",
            ToString(NewUInt64(std::numeric_limits<uint64_t>::max())));
  EXPECT_EQ("0ul", ToString(NewUInt64(std::numeric_limits<uint64_t>::min())));
}

}  // namespace optimizer
}  // namespace elang
