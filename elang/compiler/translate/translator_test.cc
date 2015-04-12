// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/translate/translate_test.h"

#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/testing/namespace_builder.h"
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

//////////////////////////////////////////////////////////////////////
//
// tests...
//

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
      "0002: control %c7 = call(%c1, %e4, void(void) System.Void Sample.Fn0(), void)\n"
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
      "0002: control %c18 = if_true(%c17)\n"
      "0003: control %c19 = br(%c18)\n"
      "0004: control %c7 = loop(%c10, %c19)\n"
      "0005: control %c15 = br(%c7)\n"
      "0006: control %c9 = merge(%c15)\n"
      "0007: int32 %r5 = param(%c1, 0)\n"
      "0008: int32 %r12 = phi(%c10: %r5, %c19: %r14)\n"
      "0009: int32 %r14 = add(%r12, 1)\n"
      "0010: int32 %r6 = param(%c1, 1)\n"
      "0011: int32 %r13 = phi(%c10: %r6, %c19: %r13)\n"
      "0012: bool %r16 = cmp_le(%r14, %r13)\n"
      "0013: control %c17 = if(%c9, %r16)\n"
      "0014: control %c20 = if_false(%c17)\n"
      "0015: control %c21 = br(%c20)\n"
      "0016: control %c8 = merge(%c21)\n"
      "0017: effect %e4 = get_effect(%c1)\n"
      "0018: effect %e11 = effect_phi(%c10: %e4, %c19: %e11)\n"
      "0019: control %c22 = ret(%c8, %e11, %r14)\n"
      "0020: control %c2 = merge(%c22)\n"
      "0021: exit(%c2)\n",
      Translate("Sample.Foo"));
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
      "0011: control %c30 = if_true(%c29)\n"
      "0012: control %c31 = br(%c30)\n"
      "0013: control %c6 = loop(%c17, %c31)\n"
      "0014: effect %e4 = get_effect(%c1)\n"
      "0015: effect %e23 = get_effect(%c22)\n"
      "0016: effect %e18 = effect_phi(%c17: %e4, %c31: %e23)\n"
      "0017: uintptr %r25 = static_cast(%r20)\n"
      "0018: uintptr %r26 = add(%r25, sizeof(char))\n"
      "0019: char* %r27 = static_cast(%r26)\n"
      "0020: char* %r20 = phi(%c17: %r9, %c31: %r27)\n"
      "0021: char %r21 = load(%e18, %r5, %r20)\n"
      "0022: control %c22 = call(%c6, %e18, void(char) System.Void Sample.Use("
      "System.Char), %r21)\n"
      "0023: control %c24 = br(%c22)\n"
      "0024: control %c7 = merge(%c24)\n"
      "0025: bool %r28 = cmp_ult(%r27, %r11)\n"
      "0026: control %c29 = if(%c7, %r28)\n"
      "0027: control %c32 = if_false(%c29)\n"
      "0028: control %c33 = br(%c32)\n"
      "0029: control %c8 = merge(%c15, %c33)\n"
      "0030: effect %e34 = effect_phi(%c15: %e4, %c33: %e23)\n"
      "0031: control %c37 = ret(%c8, %e34, void)\n"
      "0032: control %c2 = merge(%c37)\n"
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
      "0001: bool %r9 = cmp_le(0, 10)\n"
      "0002: control %c10 = if(%c1, %r9)\n"
      "0003: control %c11 = if_false(%c10)\n"
      "0004: control %c12 = br(%c11)\n"
      "0005: control %c13 = if_true(%c10)\n"
      "0006: control %c14 = br(%c13)\n"
      "0007: control %c26 = if_true(%c25)\n"
      "0008: control %c27 = br(%c26)\n"
      "0009: control %c6 = loop(%c14, %c27)\n"
      "0010: control %c23 = br(%c6)\n"
      "0011: control %c7 = merge(%c23)\n"
      "0012: int32 %r18 = phi(%c14: 0, %c27: %r21)\n"
      "0013: int32 %r5 = param(%c1, 0)\n"
      "0014: int32 %r17 = phi(%c14: %r5, %c27: %r17)\n"
      "0015: int32 %r20 = add(%r17, 1)\n"
      "0016: int32 %r21 = add(%r18, %r20)\n"
      "0017: bool %r24 = cmp_le(%r21, 10)\n"
      "0018: control %c25 = if(%c7, %r24)\n"
      "0019: control %c28 = if_false(%c25)\n"
      "0020: control %c29 = br(%c28)\n"
      "0021: control %c8 = merge(%c12, %c29)\n"
      "0022: effect %e4 = get_effect(%c1)\n"
      "0023: effect %e15 = effect_phi(%c14: %e4, %c27: %e15)\n"
      "0024: effect %e30 = effect_phi(%c12: %e4, %c29: %e15)\n"
      "0025: int32 %r22 = mul(%r21, 2)\n"
      "0026: int32 %r34 = phi(%c12: 0, %c29: %r22)\n"
      "0027: control %c35 = ret(%c8, %e30, %r34)\n"
      "0028: control %c2 = merge(%c35)\n"
      "0029: exit(%c2)\n",
      Translate("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
