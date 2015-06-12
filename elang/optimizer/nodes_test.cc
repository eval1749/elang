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
// NodesTest
//
class NodesTest : public testing::OptimizerTest {
 protected:
  NodesTest() = default;
  ~NodesTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodesTest);
};

// Test cases...

TEST_F(NodesTest, BoolNode) {
  EXPECT_EQ(false_value(), false_value());
  EXPECT_EQ(true_value(), true_value());
  EXPECT_EQ("false", ToString(false_value()));
  EXPECT_EQ("true", ToString(true_value()));
}

TEST_F(NodesTest, CallNode) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);
  auto const callee = NewReference(
      NewFunctionType(void_type(), NewTupleType({int32_type(), int64_type()})),
      NewAtomicString(L"Foo"));
  auto const arguments =
      NewTuple({NewParameter(entry_node, 0), NewParameter(entry_node, 1)});
  auto const node = NewCall(entry_node, effect, callee, arguments);
  EXPECT_EQ("control %c8 = call(%c1, %e4, void(int32, int64) Foo, %t7)",
            ToString(node));
}

TEST_F(NodesTest, CharNode) {
  EXPECT_EQ(NewChar('a'), NewChar('a'));
  EXPECT_NE(NewChar('z'), NewChar('a'));
  EXPECT_EQ("'a'", ToString(NewChar('a')));
  EXPECT_EQ("'\\''", ToString(NewChar('\'')));
}

TEST_F(NodesTest, DynamicCastNode) {
  auto const function = NewSampleFunction(void_type(), int32_type());
  auto const entry_node = function->entry_node();
  auto const node = NewDynamicCast(int64_type(), NewParameter(entry_node, 0));
  EXPECT_EQ("int64 %r5 = dynamic_cast(%r4)", ToString(node));
}

TEST_F(NodesTest, EffectPhi) {
  auto const merge_node = NewMerge({});
  auto const node = NewEffectPhi(merge_node);
  EXPECT_EQ("effect %e2 = effect_phi()", ToString(node));
}

TEST_F(NodesTest, ElementNode) {
  auto const array_pointer =
      NewReference(NewPointerType(NewArrayType(int32_type(), {-1})),
                   NewAtomicString(L"Sample.array_"));
  auto const node = NewElement(array_pointer, NewInt32(3));
  EXPECT_EQ("int32* %r1 = element(int32[]* Sample.array_, 3)", ToString(node));
}

TEST_F(NodesTest, EntryNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = function->entry_node();
  EXPECT_EQ("Validate.EntryNode.NoUsers(control %c1 = entry())\n",
            ToString(node));
}

TEST_F(NodesTest, ExitNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = static_cast<Node*>(function->exit_node());
  EXPECT_TRUE(node->IsControl());
  EXPECT_FALSE(node->IsData());
  EXPECT_FALSE(node->IsEffect());
  EXPECT_EQ("exit(%c2)", ToString(node));
}

TEST_F(NodesTest, FieldNode) {
  auto const clazz = NewExternalType(NewAtomicString(L"Sample"));
  auto const instance_pointer =
      NewReference(NewPointerType(clazz), NewAtomicString(L"this"));
  auto const node = NewField(int32_type(), instance_pointer, NewString(L"x_"));
  EXPECT_EQ("int32* %r1 = field(Sample* this, \"x_\")", ToString(node));
}

TEST_F(NodesTest, Float32Node) {
  EXPECT_EQ(NewFloat32(0), NewFloat32(0));
  EXPECT_NE(NewFloat32(1), NewFloat32(0));
  EXPECT_EQ("0.000000f", ToString(NewFloat32(0.0f)));
  EXPECT_EQ("-1.00000f", ToString(NewFloat32(-1.0f)));
  EXPECT_EQ("3.14000f", ToString(NewFloat32(3.14f)));
}

TEST_F(NodesTest, Float64Node) {
  EXPECT_EQ(NewFloat64(0), NewFloat64(0));
  EXPECT_NE(NewFloat64(1), NewFloat64(0));
  EXPECT_EQ("0.000000", ToString(NewFloat64(0)));
  EXPECT_EQ("-1.00000", ToString(NewFloat64(-1)));
  EXPECT_EQ("3.14000", ToString(NewFloat64(3.14)));
}

#define V(Name, mnemonic)                                                  \
  TEST_F(NodesTest, Float##Name##Node) {                                   \
    auto const function = NewSampleFunction(                               \
        void_type(),                                                       \
        {float32_type(), float32_type(), float64_type(), float64_type()}); \
    auto const entry_node = function->entry_node();                        \
    auto const node32 = NewFloat##Name(NewParameter(entry_node, 0),        \
                                       NewParameter(entry_node, 1));       \
    EXPECT_EQ("float32 %r6 = " mnemonic "(%r5, %r4)", ToString(node32));   \
    auto const node64 = NewFloat##Name(NewParameter(entry_node, 2),        \
                                       NewParameter(entry_node, 3));       \
    EXPECT_EQ("float64 %r9 = " mnemonic "(%r8, %r7)", ToString(node64));   \
  }
V(Add, "fadd")
V(Div, "fdiv")
V(Mod, "fmod")
V(Mul, "fmul")
V(Sub, "fsub")
#undef V

TEST_F(NodesTest, FloatCmpNode) {
  auto const function = NewSampleFunction(
      void_type(),
      {float32_type(), float32_type(), float64_type(), float64_type()});
  auto const entry_node = function->entry_node();
  auto const node32 =
      NewFloatCmp(FloatCondition::OrderedEqual, NewParameter(entry_node, 0),
                  NewParameter(entry_node, 1));
  EXPECT_EQ("bool %r6 = fcmp_eq(%r5, %r4)", ToString(node32));
  auto const node64 =
      NewFloatCmp(FloatCondition::OrderedNotEqual, NewParameter(entry_node, 2),
                  NewParameter(entry_node, 3));
  EXPECT_EQ("bool %r9 = fcmp_ne(%r8, %r7)", ToString(node64));
}

TEST_F(NodesTest, GetNode) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const tuple = NewGetTuple(entry_node);
  auto const node = NewGet(tuple, 1);
  EXPECT_EQ("int64 %r5 = get(%t4, 1)", ToString(node));
}

TEST_F(NodesTest, GetData) {
  auto const function = NewSampleFunction(void_type(), int32_type());
  auto const node = NewGetData(function->entry_node());
  EXPECT_EQ("int32 %r4 = get_data(%c1)", ToString(node));
}

TEST_F(NodesTest, GetEffect) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = NewGetEffect(function->entry_node());
  EXPECT_EQ("effect %e4 = get_effect(%c1)", ToString(node));
}

TEST_F(NodesTest, GetTuple) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const node = NewGetTuple(function->entry_node());
  EXPECT_EQ("(int32, int64) %t4 = get_tuple(%c1)", ToString(node));
}

TEST_F(NodesTest, Int8Node) {
  EXPECT_EQ(NewInt8(0), NewInt8(0));
  EXPECT_NE(NewInt8(1), NewInt8(0));
  EXPECT_EQ("int8(0)", ToString(NewInt8(0)));
  EXPECT_EQ("int8(-1)", ToString(NewInt8(-1)));
  EXPECT_EQ("int8(127)", ToString(NewInt8(std::numeric_limits<int8_t>::max())));
  EXPECT_EQ("int8(-128)",
            ToString(NewInt8(std::numeric_limits<int8_t>::min())));
}

TEST_F(NodesTest, Int16Node) {
  EXPECT_EQ(NewInt16(0), NewInt16(0));
  EXPECT_NE(NewInt16(1), NewInt16(0));
  EXPECT_EQ("int16(0)", ToString(NewInt16(0)));
  EXPECT_EQ("int16(-1)", ToString(NewInt16(-1)));
  EXPECT_EQ("int16(32767)",
            ToString(NewInt16(std::numeric_limits<int16_t>::max())));
  EXPECT_EQ("int16(-32768)",
            ToString(NewInt16(std::numeric_limits<int16_t>::min())));
}

TEST_F(NodesTest, Int32Node) {
  EXPECT_EQ(NewInt32(0), NewInt32(0));
  EXPECT_NE(NewInt32(1), NewInt32(0));
  EXPECT_EQ("0", ToString(NewInt32(0)));
  EXPECT_EQ("-1", ToString(NewInt32(-1)));
  EXPECT_EQ("2147483647",
            ToString(NewInt32(std::numeric_limits<int32_t>::max())));
  EXPECT_EQ("-2147483648",
            ToString(NewInt32(std::numeric_limits<int32_t>::min())));
}

TEST_F(NodesTest, Int64Node) {
  EXPECT_EQ(NewInt64(0), NewInt64(0));
  EXPECT_NE(NewInt64(1), NewInt64(0));
  EXPECT_EQ("0l", ToString(NewInt64(0)));
  EXPECT_EQ("-1l", ToString(NewInt64(-1)));
  EXPECT_EQ("9223372036854775807l",
            ToString(NewInt64(std::numeric_limits<int64_t>::max())));
  EXPECT_EQ("-9223372036854775808l",
            ToString(NewInt64(std::numeric_limits<int64_t>::min())));
}

#define V(Name, mnemonic)                                                    \
  TEST_F(NodesTest, Name##Node) {                                            \
    auto const function = NewSampleFunction(void_type(), {int32_type(),      \
                                                          int32_type(),      \
                                                          int64_type(),      \
                                                          int64_type(),      \
                                                          uint32_type(),     \
                                                          uint32_type(),     \
                                                          uint64_type(),     \
                                                          uint64_type()});   \
    auto const entry_node = function->entry_node();                          \
    auto const node32 =                                                      \
        New##Name(NewParameter(entry_node, 0), NewParameter(entry_node, 1)); \
    EXPECT_EQ("int32 %r6 = " mnemonic "(%r5, %r4)", ToString(node32));       \
    auto const node64 =                                                      \
        New##Name(NewParameter(entry_node, 2), NewParameter(entry_node, 3)); \
    EXPECT_EQ("int64 %r9 = " mnemonic "(%r8, %r7)", ToString(node64));       \
    auto const node32u =                                                     \
        New##Name(NewParameter(entry_node, 4), NewParameter(entry_node, 5)); \
    EXPECT_EQ("uint32 %r12 = " mnemonic "(%r11, %r10)", ToString(node32u));  \
    auto const node64u =                                                     \
        New##Name(NewParameter(entry_node, 6), NewParameter(entry_node, 7)); \
    EXPECT_EQ("uint64 %r15 = " mnemonic "(%r14, %r13)", ToString(node64u));  \
  }
V(IntAdd, "add")
V(IntBitAnd, "bit_and")
V(IntBitOr, "bit_or")
V(IntBitXor, "bit_xor")
V(IntSub, "sub")
#undef V

#define V(Name, mnemonic)                                                    \
  TEST_F(NodesTest, Name##Node) {                                            \
    auto const function = NewSampleFunction(                                 \
        void_type(),                                                         \
        {int32_type(), int32_type(), int64_type(), int64_type()});           \
    auto const entry_node = function->entry_node();                          \
    auto const node32 =                                                      \
        New##Name(NewParameter(entry_node, 0), NewParameter(entry_node, 1)); \
    EXPECT_EQ("int32 %r6 = " mnemonic "(%r5, %r4)", ToString(node32));       \
    auto const node64 =                                                      \
        New##Name(NewParameter(entry_node, 2), NewParameter(entry_node, 3)); \
    EXPECT_EQ("int64 %r9 = " mnemonic "(%r8, %r7)", ToString(node64));       \
  }
V(IntDiv, "div")
V(IntMod, "mod")
V(IntMul, "mul")
#undef V

TEST_F(NodesTest, IntCmpNode) {
  auto const function = NewSampleFunction(
      void_type(), {int32_type(), int32_type(), int64_type(), int64_type()});
  auto const entry_node = function->entry_node();
  auto const node32 =
      NewIntCmp(IntCondition::Equal, NewParameter(entry_node, 0),
                NewParameter(entry_node, 1));
  EXPECT_EQ("bool %r6 = cmp_eq(%r5, %r4)", ToString(node32));

  auto const node64 =
      NewIntCmp(IntCondition::NotEqual, NewParameter(entry_node, 2),
                NewParameter(entry_node, 3));
  EXPECT_EQ("bool %r9 = cmp_ne(%r8, %r7)", ToString(node64));
}

TEST_F(NodesTest, IntCmpNodePointerType) {
  auto const function = NewSampleFunction(
      void_type(),
      {NewPointerType(int32_type()), NewPointerType(int32_type())});
  auto const entry_node = function->entry_node();
  auto const node = NewIntCmp(IntCondition::Equal, NewParameter(entry_node, 0),
                              NewParameter(entry_node, 1));
  EXPECT_EQ("bool %r6 = cmp_eq(%r5, %r4)", ToString(node));
}

#define V(Name, mnemonic)                                                    \
  TEST_F(NodesTest, Name##Node) {                                            \
    auto const function = NewSampleFunction(                                 \
        void_type(),                                                         \
        {int32_type(), int64_type(), uint32_type(), uint64_type()});         \
    auto const entry_node = function->entry_node();                          \
    auto const node32 =                                                      \
        New##Name(NewParameter(entry_node, 0), NewParameter(entry_node, 0)); \
    EXPECT_EQ("int32 %r6 = " mnemonic "(%r5, %r4)", ToString(node32));       \
    auto const node64 =                                                      \
        New##Name(NewParameter(entry_node, 1), NewParameter(entry_node, 0)); \
    EXPECT_EQ("int64 %r9 = " mnemonic "(%r8, %r7)", ToString(node64));       \
    auto const node32u =                                                     \
        New##Name(NewParameter(entry_node, 2), NewParameter(entry_node, 0)); \
    EXPECT_EQ("uint32 %r12 = " mnemonic "(%r11, %r10)", ToString(node32u));  \
    auto const node64u =                                                     \
        New##Name(NewParameter(entry_node, 3), NewParameter(entry_node, 0)); \
    EXPECT_EQ("uint64 %r15 = " mnemonic "(%r14, %r13)", ToString(node64u));  \
  }
V(IntShl, "shl")
V(IntShr, "shr")
#undef V

TEST_F(NodesTest, LengthNode) {
  auto const array_pointer =
      NewReference(NewPointerType(NewArrayType(char_type(), {-1})),
                   NewAtomicString(L"Sample.array_"));
  auto const node = NewLength(array_pointer, 0);
  EXPECT_EQ("int32 %r1 = length(char[]* Sample.array_, 0)", ToString(node));
}

TEST_F(NodesTest, LoadNode) {
  auto const function =
      NewSampleFunction(void_type(), NewPointerType(char_type()));
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);
  auto const param = NewParameter(entry_node, 0);
  auto const node = NewLoad(effect, param, param);
  EXPECT_EQ("char %r6 = load(%e4, %r5, %r5)", ToString(node));
}

TEST_F(NodesTest, JumpNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const node = NewJump(function->entry_node());
  EXPECT_EQ("control %c4 = br(%c1)", ToString(node));
}

TEST_F(NodesTest, LoopNode) {
  auto const node = NewLoop();
  EXPECT_EQ("control %c1 = loop()", ToString(node));
}

TEST_F(NodesTest, ParameterNode) {
  auto const function = NewSampleFunction(void_type(), int32_type());
  auto const entry_node = function->entry_node();
  auto const node = NewParameter(entry_node, 0);
  EXPECT_EQ("int32 %r4 = param(%c1, 0)", ToString(node));
}

TEST_F(NodesTest, ParameterNode2) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const node = NewParameter(entry_node, 1);
  EXPECT_EQ("int64 %r4 = param(%c1, 1)", ToString(node));
}

TEST_F(NodesTest, ReferenceNode) {
  auto const node = NewReference(NewFunctionType(void_type(), int32_type()),
                                 NewAtomicString(L"Foo"));
  EXPECT_EQ("void(int32) Foo", ToString(node));
}

TEST_F(NodesTest, RetNode) {
  auto const function = NewSampleFunction(void_type(), void_type());
  auto const entry_node = function->entry_node();
  auto const node = NewRet(entry_node, NewGetEffect(entry_node), void_value());
  EXPECT_EQ("control %c5 = ret(%c1, %e4, void)", ToString(node));
}

TEST_F(NodesTest, SizeOfNode) {
  auto const node = NewSizeOf(intptr_type());
  auto const node2 = NewSizeOf(intptr_type());
  EXPECT_EQ(node, node2);
  EXPECT_EQ("sizeof(intptr)", ToString(node));
}

TEST_F(NodesTest, StoreNode) {
  auto const function =
      NewSampleFunction(void_type(), NewPointerType(char_type()));
  auto const entry_node = function->entry_node();
  auto const effect = NewGetEffect(entry_node);
  auto const param = NewParameter(entry_node, 0);
  auto const node = NewStore(effect, param, param, NewChar('a'));
  EXPECT_EQ("effect %e6 = store(%e4, %r5, %r5, 'a')", ToString(node));
}

TEST_F(NodesTest, StaticCastNode) {
  auto const function = NewSampleFunction(void_type(), int32_type());
  auto const entry_node = function->entry_node();
  auto const node = NewStaticCast(int64_type(), NewParameter(entry_node, 0));
  EXPECT_EQ("int64 %r5 = static_cast(%r4)", ToString(node));
}

TEST_F(NodesTest, StringNode) {
  EXPECT_EQ("\"abc\"", ToString(NewString(L"abc")));
  EXPECT_EQ("\"123\\n456\"", ToString(NewString(L"123\n456")));
  EXPECT_EQ("\"\\u1234\"", ToString(NewString(L"\u1234")));
}

TEST_F(NodesTest, TupleNode) {
  auto const function = NewSampleFunction(
      void_type(), NewTupleType({int32_type(), int64_type()}));
  auto const entry_node = function->entry_node();
  auto const parameter0 = NewParameter(entry_node, 0);
  auto const parameter1 = NewParameter(entry_node, 1);
  auto const node = NewTuple({parameter1, parameter0});
  EXPECT_EQ("(int64, int32) %t6 = tuple(%r5, %r4)", ToString(node));
}

TEST_F(NodesTest, UInt8Node) {
  EXPECT_EQ(NewUInt8(0), NewUInt8(0));
  EXPECT_NE(NewUInt8(1), NewUInt8(0));
  EXPECT_EQ("uint8(0)", ToString(NewUInt8(0)));
  EXPECT_EQ("uint8(255)",
            ToString(NewUInt8(std::numeric_limits<uint8_t>::max())));
  EXPECT_EQ("uint8(0)",
            ToString(NewUInt8(std::numeric_limits<uint8_t>::min())));
}

TEST_F(NodesTest, UInt16Node) {
  EXPECT_EQ(NewUInt16(0), NewUInt16(0));
  EXPECT_NE(NewUInt16(1), NewUInt16(0));
  EXPECT_EQ("uint16(0)", ToString(NewUInt16(0)));
  EXPECT_EQ("uint16(65535)",
            ToString(NewUInt16(std::numeric_limits<uint16_t>::max())));
  EXPECT_EQ("uint16(0)",
            ToString(NewUInt16(std::numeric_limits<uint16_t>::min())));
}

TEST_F(NodesTest, UInt32Node) {
  EXPECT_EQ(NewUInt32(0), NewUInt32(0));
  EXPECT_NE(NewUInt32(1), NewUInt32(0));
  EXPECT_EQ("0u", ToString(NewUInt32(0)));
  EXPECT_EQ("4294967295u",
            ToString(NewUInt32(std::numeric_limits<uint32_t>::max())));
  EXPECT_EQ("0u", ToString(NewUInt32(std::numeric_limits<uint32_t>::min())));
}

TEST_F(NodesTest, UInt64Node) {
  EXPECT_EQ(NewUInt64(0), NewUInt64(0));
  EXPECT_NE(NewUInt64(1), NewUInt64(0));
  EXPECT_EQ("0ul", ToString(NewUInt64(0)));
  EXPECT_EQ("18446744073709551615ul",
            ToString(NewUInt64(std::numeric_limits<uint64_t>::max())));
  EXPECT_EQ("0ul", ToString(NewUInt64(std::numeric_limits<uint64_t>::min())));
}

#define V(Name, mnemonic)                                                    \
  TEST_F(NodesTest, Name##Node) {                                            \
    auto const function = NewSampleFunction(                                 \
        void_type(),                                                         \
        {uint32_type(), uint32_type(), uint64_type(), uint64_type()});       \
    auto const entry_node = function->entry_node();                          \
    auto const node32u =                                                     \
        New##Name(NewParameter(entry_node, 0), NewParameter(entry_node, 1)); \
    EXPECT_EQ("uint32 %r6 = " mnemonic "(%r5, %r4)", ToString(node32u));     \
    auto const node64u =                                                     \
        New##Name(NewParameter(entry_node, 2), NewParameter(entry_node, 3)); \
    EXPECT_EQ("uint64 %r9 = " mnemonic "(%r8, %r7)", ToString(node64u));     \
  }
V(UIntDiv, "udiv")
V(UIntMod, "umod")
V(UIntMul, "umul")
#undef V

}  // namespace optimizer
}  // namespace elang
