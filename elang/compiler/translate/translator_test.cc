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
      "0000: (control, effect, (int32, int32)) %t1 = entry()\n"
      "0001: control %c4 = control_get(%t1, 0)\n"
      "0002: effect %e5 = effect_get(%t1, 1)\n"
      "0003: (control, effect, void) %t8 = call(%c4, %e5, void(void) "
      "System.Void Sample.Fn0(), void)\n"
      "0004: control %c10 = control_get(%t8, 0)\n"
      "0005: effect %e9 = effect_get(%t8, 1)\n"
      "0006: int32 %r6 = param(%t1, 0)\n"
      "0007: (control, effect, int32) %t12 = call(%c10, %e9, int32(int32) "
      "System.Int32 Sample.Fn1(System.Int32), %r6)\n"
      "0008: control %c14 = control_get(%t12, 0)\n"
      "0009: effect %e13 = effect_get(%t12, 1)\n"
      "0010: int32 %r7 = param(%t1, 1)\n"
      "0011: (int32, int32) %t16 = tuple(%r6, %r7)\n"
      "0012: (control, effect, int32) %t17 = call(%c14, %e13, int32(int32, "
      "int32) System.Int32 Sample.Fn2(System.Int32, System.Int32), %t16)\n"
      "0013: control %c19 = control_get(%t17, 0)\n"
      "0014: effect %e18 = effect_get(%t17, 1)\n"
      "0015: control %c21 = ret(%c19, %e18, void)\n"
      "0016: control %c2 = merge(%c21)\n"
      "0017: void %r3 = exit(%c2)\n",
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
      "0000: (control, effect, (int32, int32)) %t1 = entry()\n"
      "0001: control %c4 = control_get(%t1, 0)\n"
      "0002: control %c11 = br(%c4)\n"
      "0003: control %c19 = if_true(%c18)\n"
      "0004: control %c20 = br(%c19)\n"
      "0005: control %c10 = merge(%c11, %c20)\n"
      "0006: control %c16 = br(%c10)\n"
      "0007: control %c9 = merge(%c16)\n"
      "0008: int32 %r6 = param(%t1, 0)\n"
      "0009: int32 %r13 = phi(%c11: %r6, %c20: %r15)\n"
      "0010: int32 %r15 = add(%r13, 1)\n"
      "0011: int32 %r7 = param(%t1, 1)\n"
      "0012: int32 %r14 = phi(%c11: %r7, %c20: %r14)\n"
      "0013: bool %r17 = cmp_le(%r15, %r14)\n"
      "0014: control %c18 = if(%c9, %r17)\n"
      "0015: control %c21 = if_false(%c18)\n"
      "0016: control %c22 = br(%c21)\n"
      "0017: control %c8 = merge(%c22)\n"
      "0018: effect %e5 = effect_get(%t1, 1)\n"
      "0019: effect %e12 = effect_phi(%c11: %e5, %c20: %e12)\n"
      "0020: control %c23 = ret(%c8, %e12, %r15)\n"
      "0021: control %c2 = merge(%c23)\n"
      "0022: void %r3 = exit(%c2)\n",
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
      "0000: (control, effect, char[]*) %t1 = entry()\n"
      "0001: control %c4 = control_get(%t1, 0)\n"
      "0002: control %c13 = br(%c4)\n"
      "0003: control %c31 = if_true(%c30)\n"
      "0004: control %c32 = br(%c31)\n"
      "0005: control %c16 = loop(%c32)\n"
      "0006: effect %e5 = effect_get(%t1, 1)\n"
      "0007: effect %e19 = effect_get(%t18, 1)\n"
      "0008: effect %e15 = effect_phi(%c13: %e5, %c26: %e19)\n"
      "0009: char[]* %r6 = param(%t1, 0)\n"
      "0010: char* %r10 = element(%r6, 0)\n"
      "0011: uintptr %r23 = static_cast(%r14)\n"
      "0012: uintptr %r24 = add(%r23, sizeof(char))\n"
      "0013: char* %r25 = static_cast(%r24)\n"
      "0014: char* %r14 = phi(%c13: %r10, %c26: %r25)\n"
      "0015: char %r17 = load(%e15, %r6, %r14)\n"
      "0016: (control, effect, void) %t18 = call(%c16, %e15, void(char) "
      "System.Void Sample.Use(System.Char), %r17)\n"
      "0017: control %c20 = control_get(%t18, 0)\n"
      "0018: control %c22 = br(%c20)\n"
      "0019: control %c7 = merge(%c22)\n"
      "0020: control %c26 = br(%c7)\n"
      "0021: control %c8 = merge(%c13, %c26)\n"
      "0022: uintptr %r27 = static_cast(%r14)\n"
      "0023: int32 %r11 = length(%r6, 0)\n"
      "0024: char* %r12 = element(%r6, %r11)\n"
      "0025: uintptr %r28 = static_cast(%r12)\n"
      "0026: bool %r29 = cmp_ult(%r27, %r28)\n"
      "0027: control %c30 = if(%c8, %r29)\n"
      "0028: control %c33 = if_false(%c30)\n"
      "0029: control %c34 = br(%c33)\n"
      "0030: control %c9 = merge(%c34)\n"
      "0031: control %c35 = ret(%c9, %e15, void)\n"
      "0032: control %c2 = merge(%c35)\n"
      "0033: void %r3 = exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, IntMul) {
  Prepare(
      "class Sample {"
      "  static int Foo(int a, int16 b) { return a * b; }"
      "}");
  EXPECT_EQ(
      "function1 int32(int32, int16)\n"
      "0000: (control, effect, (int32, int16)) %t1 = entry()\n"
      "0001: control %c4 = control_get(%t1, 0)\n"
      "0002: effect %e5 = effect_get(%t1, 1)\n"
      "0003: int32 %r6 = param(%t1, 0)\n"
      "0004: int16 %r7 = param(%t1, 1)\n"
      "0005: int32 %r8 = static_cast(%r7)\n"
      "0006: int32 %r9 = mul(%r6, %r8)\n"
      "0007: control %c10 = ret(%c4, %e5, %r9)\n"
      "0008: control %c2 = merge(%c10)\n"
      "0009: void %r3 = exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, IntCmp) {
  Prepare(
      "class Sample {"
      "  static bool Foo(int a, int b) { return a < b; }"
      "}");
  EXPECT_EQ(
      "function1 bool(int32, int32)\n"
      "0000: (control, effect, (int32, int32)) %t1 = entry()\n"
      "0001: control %c4 = control_get(%t1, 0)\n"
      "0002: effect %e5 = effect_get(%t1, 1)\n"
      "0003: int32 %r6 = param(%t1, 0)\n"
      "0004: int32 %r7 = param(%t1, 1)\n"
      "0005: bool %r8 = cmp_le(%r6, %r7)\n"
      "0006: control %c9 = ret(%c4, %e5, %r8)\n"
      "0007: control %c2 = merge(%c9)\n"
      "0008: void %r3 = exit(%c2)\n",
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
      "0000: (control, effect, bool) %t1 = entry()\n"
      "0001: control %c4 = control_get(%t1, 0)\n"
      "0002: bool %r6 = param(%t1, 0)\n"
      "0003: control %c7 = if(%c4, %r6)\n"
      "0004: control %c9 = if_true(%c7)\n"
      "0005: control %c10 = br(%c9)\n"
      "0006: control %c11 = if_false(%c7)\n"
      "0007: control %c12 = br(%c11)\n"
      "0008: control %c8 = merge(%c10, %c12)\n"
      "0009: effect %e5 = effect_get(%t1, 1)\n"
      "0010: int32 %r13 = phi(%c10: 1, %c12: 0)\n"
      "0011: control %c14 = ret(%c8, %e5, %r13)\n"
      "0012: control %c2 = merge(%c14)\n"
      "0013: void %r3 = exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, ReturnLiteral) {
  Prepare(
      "class Sample {"
      "  static int Foo() { return 123; }"
      "}");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "0000: (control, effect, void) %t1 = entry()\n"
      "0001: control %c4 = control_get(%t1, 0)\n"
      "0002: effect %e5 = effect_get(%t1, 1)\n"
      "0003: control %c6 = ret(%c4, %e5, 123)\n"
      "0004: control %c2 = merge(%c6)\n"
      "0005: void %r3 = exit(%c2)\n",
      Translate("Sample.Foo"));
}

TEST_F(TranslatorTest, ReturnParameter) {
  Prepare(
      "class Sample {"
      "  static int Foo(int x) { return x; }"
      "}");
  EXPECT_EQ(
      "function1 int32(int32)\n"
      "0000: (control, effect, int32) %t1 = entry()\n"
      "0001: control %c4 = control_get(%t1, 0)\n"
      "0002: effect %e5 = effect_get(%t1, 1)\n"
      "0003: int32 %r6 = param(%t1, 0)\n"
      "0004: control %c7 = ret(%c4, %e5, %r6)\n"
      "0005: control %c2 = merge(%c7)\n"
      "0006: void %r3 = exit(%c2)\n",
      Translate("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
