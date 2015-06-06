// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/translate/translate_test.h"

#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/namespace_builder.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/translate/type_mapper.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/type_factory.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// TranslatorTest
//
class TranslatorTest : public testing::TranslateTest {
 protected:
  TranslatorTest() = default;
  ~TranslatorTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TranslatorTest);
};

TEST_F(TranslatorTest, ArrayAccessLoad) {
  Prepare(
      "class Sample {"
      "  static int Foo(int[] arr) { return arr[1]; }"
      "}");
  EXPECT_EQ(
      "function1 int32(int32[]*)\n"
      "0000: control(int32[]*) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: int32[]* %r5 = param(%c1, 0)\n"
      "0003: int32* %r6 = element(%r5, 1)\n"
      "0004: int32 %r7 = load(%e4, %r5, %r6)\n"
      "0005: control %c8 = ret(%c1, %e4, %r7)\n"
      "0006: control %c2 = merge(%c8)\n"
      "0007: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, ArrayAccessStore) {
  Prepare(
      "class Sample {"
      "  static void Foo(int[] arr) { arr[1] = 42; }"
      "}");
  EXPECT_EQ(
      "function1 void(int32[]*)\n"
      "0000: control(int32[]*) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: int32[]* %r5 = param(%c1, 0)\n"
      "0003: int32* %r6 = element(%r5, 1)\n"
      "0004: effect %e7 = store(%e4, %r5, %r6, 42)\n"
      "0005: control %c8 = ret(%c1, %e7, void)\n"
      "0006: control %c2 = merge(%c8)\n"
      "0007: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, Calls) {
  Prepare(
      "class Sample {"
      "  static void Foo(int a, int b) { Fn0(); Fn1(a); Fn2(a, b); }"
      "  static void Fn0() {}"
      "  static int Fn1(int a) { return a; }"
      "  static int Fn2(int a, int b) {return a + b; }"
      "}");
  EXPECT_EQ(
      "function1 void(int32, int32)\n"
      "0000: control((int32, int32)) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: control %c7 = call(%c1, %e4, void(void) System.Void Sample.Fn0(), "
      "void)\n"
      "0003: effect %e8 = get_effect(%c7)\n"
      "0004: int32 %r5 = param(%c1, 0)\n"
      "0005: control(int32) %c9 = call(%c7, %e8, int32(int32) System.Int32 "
      "Sample.Fn1(System.Int32), %r5)\n"
      "0006: effect %e10 = get_effect(%c9)\n"
      "0007: int32 %r6 = param(%c1, 1)\n"
      "0008: (int32, int32) %t12 = tuple(%r5, %r6)\n"
      "0009: control(int32) %c13 = call(%c9, %e10, int32(int32, int32) "
      "System.Int32 Sample.Fn2(System.Int32, System.Int32), %t12)\n"
      "0010: effect %e14 = get_effect(%c13)\n"
      "0011: control %c16 = ret(%c13, %e14, void)\n"
      "0012: control %c2 = merge(%c16)\n"
      "0013: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, DoWhile) {
  Prepare(
      "class Sample {"
      "  static int Foo(int a, int b) {"
      "    do { a = a + 1; } while (a < b);"
      "    return a;"
      "  }"
      "}");
  EXPECT_EQ(
      "function1 int32(int32, int32)\n"
      "0000: control((int32, int32)) %c1 = entry()\n"
      "0001: control %c10 = br(%c1)\n"
      "0002: control %c17 = if_true(%c16)\n"
      "0003: control %c18 = br(%c17)\n"
      "0004: control %c7 = loop(%c10, %c18)\n"
      "0005: control %c14 = br(%c7)\n"
      "0006: control %c9 = merge(%c14)\n"
      "0007: int32 %r5 = param(%c1, 0)\n"
      "0008: int32 %r12 = phi(%c10: %r5, %c18: %r13)\n"
      "0009: int32 %r13 = add(%r12, 1)\n"
      "0010: int32 %r6 = param(%c1, 1)\n"
      "0011: bool %r15 = cmp_le(%r13, %r6)\n"
      "0012: control %c16 = if(%c9, %r15)\n"
      "0013: control %c19 = if_false(%c16)\n"
      "0014: control %c20 = br(%c19)\n"
      "0015: control %c8 = merge(%c20)\n"
      "0016: effect %e4 = get_effect(%c1)\n"
      "0017: effect %e11 = effect_phi(%c10: %e4, %c18: %e11)\n"
      "0018: control %c21 = ret(%c8, %e11, %r13)\n"
      "0019: control %c2 = merge(%c21)\n"
      "0020: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, FieldInstanceRead) {
  Prepare(
      "class Sample {"
      "  int length_;"
      "  int Length() { return length_; }"
      "}");
  EXPECT_EQ(
      "function1 int32(Sample*)\n"
      "0000: control(Sample*) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: Sample* %r5 = param(%c1, 0)\n"
      "0003: int32* %r6 = field(%r5, int32 Sample.length_)\n"
      "0004: int32 %r7 = load(%e4, %r5, %r6)\n"
      "0005: control %c8 = ret(%c1, %e4, %r7)\n"
      "0006: control %c2 = merge(%c8)\n"
      "0007: exit(%c2)\n",
      Translate("Sample.Length"));
}

TEST_F(TranslatorTest, FieldInstanceWrite) {
  Prepare(
      "class Sample {"
      "  int length_;"
      "  void SetLength(int new_value) { length_ = new_value; }"
      "}");
  EXPECT_EQ(
      "function1 void(Sample*, int32)\n"
      "0000: control((Sample*, int32)) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: Sample* %r6 = param(%c1, 0)\n"
      "0003: int32* %r7 = field(%r6, int32 Sample.length_)\n"
      "0004: int32 %r5 = param(%c1, 1)\n"
      "0005: effect %e8 = store(%e4, %r6, %r7, %r5)\n"
      "0006: control %c9 = ret(%c1, %e8, void)\n"
      "0007: control %c2 = merge(%c9)\n"
      "0008: exit(%c2)\n",
      Translate("Sample.SetLength"));
}

TEST_F(TranslatorTest, FieldStaticRead) {
  Prepare(
      "class Sample {"
      "  static int length_;"
      "  int Length() { return length_; }"
      "}");
  EXPECT_EQ(
      "function1 int32(Sample*)\n"
      "0000: control(Sample*) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: int32 %r5 = load(%e4, int32* Sample.length_, int32* "
      "Sample.length_)\n"
      "0003: control %c6 = ret(%c1, %e4, %r5)\n"
      "0004: control %c2 = merge(%c6)\n"
      "0005: exit(%c2)\n",
      Translate("Sample.Length"));
}

TEST_F(TranslatorTest, FieldStaticWrite) {
  Prepare(
      "class Sample {"
      "  static int length_;"
      "  void SetLength(int new_value) { length_ = new_value; }"
      "}");
  EXPECT_EQ(
      "function1 void(Sample*, int32)\n"
      "0000: control((Sample*, int32)) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: int32 %r5 = param(%c1, 1)\n"
      "0003: effect %e6 = store(%e4, int32* Sample.length_, int32* "
      "Sample.length_, %r5)\n"
      "0004: control %c7 = ret(%c1, %e6, void)\n"
      "0005: control %c2 = merge(%c7)\n"
      "0006: exit(%c2)\n",
      Translate("Sample.SetLength"));
}

TEST_F(TranslatorTest, For) {
  Prepare(
      "class Sample {"
      "  static void Foo() {"
      "    for (var a = 0; Cond(a); a = Step(a))"
      "      Use(a);"
      "  }"
      "  static extern bool Cond(int x);"
      "  static extern int Step(int x);"
      "  static extern void Use(int x);"
      "}");
  EXPECT_EQ(
      "function1 void(void)\n"
      "0000: control %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: control(bool) %c8 = call(%c1, %e4, bool(int32) System.Bool "
      "Sample.Cond(System.Int32), 0)\n"
      "0003: bool %r10 = get_data(%c8)\n"
      "0004: control %c11 = if(%c8, %r10)\n"
      "0005: control %c12 = if_false(%c11)\n"
      "0006: control %c13 = br(%c12)\n"
      "0007: control %c14 = if_true(%c11)\n"
      "0008: control %c15 = br(%c14)\n"
      "0009: control %c28 = if_true(%c27)\n"
      "0010: control %c29 = br(%c28)\n"
      "0011: control %c5 = loop(%c15, %c29)\n"
      "0012: effect %e9 = get_effect(%c8)\n"
      "0013: effect %e25 = get_effect(%c24)\n"
      "0014: effect %e16 = effect_phi(%c15: %e9, %c29: %e25)\n"
      "0015: int32 %r23 = get_data(%c21)\n"
      "0016: int32 %r17 = phi(%c15: 0, %c29: %r23)\n"
      "0017: control %c18 = call(%c5, %e16, void(int32) System.Void "
      "Sample.Use(System.Int32), %r17)\n"
      "0018: control %c20 = br(%c18)\n"
      "0019: control %c6 = merge(%c20)\n"
      "0020: effect %e19 = get_effect(%c18)\n"
      "0021: control(int32) %c21 = call(%c6, %e19, int32(int32) System.Int32 "
      "Sample.Step(System.Int32), %r17)\n"
      "0022: effect %e22 = get_effect(%c21)\n"
      "0023: control(bool) %c24 = call(%c21, %e22, bool(int32) System.Bool "
      "Sample.Cond(System.Int32), %r23)\n"
      "0024: bool %r26 = get_data(%c24)\n"
      "0025: control %c27 = if(%c24, %r26)\n"
      "0026: control %c30 = if_false(%c27)\n"
      "0027: control %c31 = br(%c30)\n"
      "0028: control %c7 = merge(%c13, %c31)\n"
      "0029: effect %e32 = effect_phi(%c13: %e9, %c31: %e25)\n"
      "0030: control %c34 = ret(%c7, %e32, void)\n"
      "0031: control %c2 = merge(%c34)\n"
      "0032: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, ForEach) {
  Prepare(
      "class Sample {"
      "  static void Foo(char[] args) {"
      "    for (var arg : args)"
      "      Use(arg);"
      "  }"
      "  static extern void Use(char x);"
      "}");
  EXPECT_EQ(
      "function1 void(char[]*)\n"
      "0000: control(char[]*) %c1 = entry()\n"
      "0001: char[]* %r5 = param(%c1, 0)\n"
      "0002: char* %r9 = element(%r5, 0)\n"
      "0003: int32 %r10 = length(%r5, 0)\n"
      "0004: char* %r11 = element(%r5, %r10)\n"
      "0005: bool %r12 = cmp_ult(%r9, %r11)\n"
      "0006: control %c13 = if(%c1, %r12)\n"
      "0007: control %c14 = if_false(%c13)\n"
      "0008: control %c15 = br(%c14)\n"
      "0009: control %c16 = if_true(%c13)\n"
      "0010: control %c17 = br(%c16)\n"
      "0011: control %c29 = if_true(%c28)\n"
      "0012: control %c30 = br(%c29)\n"
      "0013: control %c6 = loop(%c17, %c30)\n"
      "0014: effect %e4 = get_effect(%c1)\n"
      "0015: effect %e22 = get_effect(%c21)\n"
      "0016: effect %e18 = effect_phi(%c17: %e4, %c30: %e22)\n"
      "0017: uintptr %r24 = static_cast(%r19)\n"
      "0018: uintptr %r25 = add(%r24, sizeof(char))\n"
      "0019: char* %r26 = static_cast(%r25)\n"
      "0020: char* %r19 = phi(%c17: %r9, %c30: %r26)\n"
      "0021: char %r20 = load(%e18, %r5, %r19)\n"
      "0022: control %c21 = call(%c6, %e18, void(char) System.Void "
      "Sample.Use(System.Char), %r20)\n"
      "0023: control %c23 = br(%c21)\n"
      "0024: control %c7 = merge(%c23)\n"
      "0025: bool %r27 = cmp_ult(%r26, %r11)\n"
      "0026: control %c28 = if(%c7, %r27)\n"
      "0027: control %c31 = if_false(%c28)\n"
      "0028: control %c32 = br(%c31)\n"
      "0029: control %c8 = merge(%c15, %c32)\n"
      "0030: effect %e33 = effect_phi(%c15: %e4, %c32: %e22)\n"
      "0031: control %c35 = ret(%c8, %e33, void)\n"
      "0032: control %c2 = merge(%c35)\n"
      "0033: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, IntMul) {
  Prepare(
      "class Sample {"
      "  static int Foo(int a, int16 b) { return a * b; }"
      "}");
  EXPECT_EQ(
      "function1 int32(int32, int16)\n"
      "0000: control((int32, int16)) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: int32 %r5 = param(%c1, 0)\n"
      "0003: int16 %r6 = param(%c1, 1)\n"
      "0004: int32 %r7 = static_cast(%r6)\n"
      "0005: int32 %r8 = mul(%r5, %r7)\n"
      "0006: control %c9 = ret(%c1, %e4, %r8)\n"
      "0007: control %c2 = merge(%c9)\n"
      "0008: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, IntCmp) {
  Prepare(
      "class Sample {"
      "  static bool Foo(int a, int b) { return a < b; }"
      "}");
  EXPECT_EQ(
      "function1 bool(int32, int32)\n"
      "0000: control((int32, int32)) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: int32 %r5 = param(%c1, 0)\n"
      "0003: int32 %r6 = param(%c1, 1)\n"
      "0004: bool %r7 = cmp_le(%r5, %r6)\n"
      "0005: control %c8 = ret(%c1, %e4, %r7)\n"
      "0006: control %c2 = merge(%c8)\n"
      "0007: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, IfMerge) {
  Prepare(
      "class Sample {"
      "  static int Foo(bool a) {"
      "    var b = 0;"
      "    if (a) b = 1;"
      "    return b;"
      "  }"
      "}");
  EXPECT_EQ(
      "function1 int32(bool)\n"
      "0000: control(bool) %c1 = entry()\n"
      "0001: bool %r5 = param(%c1, 0)\n"
      "0002: control %c6 = if(%c1, %r5)\n"
      "0003: control %c8 = if_true(%c6)\n"
      "0004: control %c9 = br(%c8)\n"
      "0005: control %c10 = if_false(%c6)\n"
      "0006: control %c11 = br(%c10)\n"
      "0007: control %c7 = merge(%c9, %c11)\n"
      "0008: effect %e4 = get_effect(%c1)\n"
      "0009: int32 %r12 = phi(%c9: 1, %c11: 0)\n"
      "0010: control %c13 = ret(%c7, %e4, %r12)\n"
      "0011: control %c2 = merge(%c13)\n"
      "0012: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, ReturnLiteral) {
  Prepare(
      "class Sample {"
      "  static int Foo() { return 123; }"
      "}");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "0000: control %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: control %c5 = ret(%c1, %e4, 123)\n"
      "0003: control %c2 = merge(%c5)\n"
      "0004: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, ReturnParameter) {
  Prepare(
      "class Sample {"
      "  static int Foo(int x) { return x; }"
      "}");
  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: control(int32) %c1 = entry()\n"
      "0001: effect %e4 = get_effect(%c1)\n"
      "0002: int32 %r5 = param(%c1, 0)\n"
      "0003: control %c6 = ret(%c1, %e4, %r5)\n"
      "0004: control %c2 = merge(%c6)\n"
      "0005: exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, While) {
  Prepare(
      "class Sample {"
      "  static int Foo(int x) {"
      "    var i = 0;"
      "    var a = x;"
      "    var c = 0;"
      "    while (i < 10) {"
      "      var b = a + 1;"
      "      i = i + b;"
      "      c = i * 2;"
      "    }"
      "    return c;"
      "  }"
      "}");
  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: control(int32) %c1 = entry()\n"
      "0001: control %c9 = br(%c1)\n"
      "0002: control %c19 = if_true(%c18)\n"
      "0003: control %c20 = br(%c19)\n"
      "0004: control %c6 = loop(%c9, %c20)\n"
      "0005: control %c16 = br(%c6)\n"
      "0006: control %c7 = merge(%c16)\n"
      "0007: int32 %r11 = phi(%c9: 0, %c20: %r14)\n"
      "0008: int32 %r5 = param(%c1, 0)\n"
      "0009: int32 %r13 = add(%r5, 1)\n"
      "0010: int32 %r14 = add(%r11, %r13)\n"
      "0011: bool %r17 = cmp_le(%r14, 10)\n"
      "0012: control %c18 = if(%c7, %r17)\n"
      "0013: control %c21 = if_false(%c18)\n"
      "0014: control %c22 = br(%c21)\n"
      "0015: control %c8 = merge(%c22)\n"
      "0016: effect %e4 = get_effect(%c1)\n"
      "0017: effect %e10 = effect_phi(%c9: %e4, %c20: %e10)\n"
      "0018: int32 %r15 = mul(%r14, 2)\n"
      "0019: control %c23 = ret(%c8, %e10, %r15)\n"
      "0020: control %c2 = merge(%c23)\n"
      "0021: exit(%c2)\n",
      Translate("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
