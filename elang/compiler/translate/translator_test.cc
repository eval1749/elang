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
      "0001: control %c2 = control_get(%t1, 0)\n"
      "0002: effect %e3 = effect_get(%t1, 1)\n"
      "0003: (effect, void) %t8 = call(%e3, void(void) System.Void "
      "Sample.Fn0(), void)\n"
      "0004: effect %e9 = effect_get(%t8, 0)\n"
      "0005: int32 %r6 = param(%t1, 0)\n"
      "0006: (effect, int32) %t10 = call(%e9, int32(int32) System.Int32 "
      "Sample.Fn1(System.Int32), %r6)\n"
      "0007: effect %e11 = effect_get(%t10, 0)\n"
      "0008: int32 %r7 = param(%t1, 1)\n"
      "0009: (int32, int32) %t12 = tuple(%r6, %r7)\n"
      "0010: (effect, int32) %t13 = call(%e11, int32(int32, int32) "
      "System.Int32 Sample.Fn2(System.Int32, System.Int32), %t12)\n"
      "0011: effect %e14 = effect_get(%t13, 0)\n"
      "0012: control %c15 = ret(%c2, %e14, void)\n"
      "0013: control %c4 = merge(%c15)\n"
      "0014: void %r5 = exit(%c4)\n",
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
      "0001: control %c2 = control_get(%t1, 0)\n"
      "0002: control %c11 = br(%c2)\n"
      "0003: control %c22 = if_true(%c21)\n"
      "0004: control %c23 = br(%c22)\n"
      "0005: control %c10 = merge(%c11, %c23)\n"
      "0006: control %c19 = br(%c10)\n"
      "0007: control %c9 = merge(%c19)\n"
      "0008: int32 %r6 = param(%t1, 0)\n"
      "0009: (control, int32) %t15 = phi_operand(%c11, %r6)\n"
      "0010: (control, int32) %t24 = phi_operand(%c23, %r18)\n"
      "0011: int32 %r14 = phi(%t15, %t24)\n"
      "0012: int32 %r18 = add(%r14, 1)\n"
      "0013: int32 %r7 = param(%t1, 1)\n"
      "0014: (control, int32) %t17 = phi_operand(%c11, %r7)\n"
      "0015: (control, int32) %t25 = phi_operand(%c23, %r16)\n"
      "0016: int32 %r16 = phi(%t17, %t25)\n"
      "0017: bool %r20 = cmp_le(%r18, %r16)\n"
      "0018: control %c21 = if(%c9, %r20)\n"
      "0019: control %c26 = if_false(%c21)\n"
      "0020: control %c27 = br(%c26)\n"
      "0021: control %c8 = merge(%c27)\n"
      "0022: effect %e3 = effect_get(%t1, 1)\n"
      "0023: (control, effect) %t13 = phi_operand(%c11, %e3)\n"
      "0024: effect %e12 = effect_phi(%t13)\n"
      "0025: control %c28 = ret(%c8, %e12, %r18)\n"
      "0026: control %c4 = merge(%c28)\n"
      "0027: void %r5 = exit(%c4)\n",
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
      "0001: control %c2 = control_get(%t1, 0)\n"
      "0002: effect %e3 = effect_get(%t1, 1)\n"
      "0003: int32 %r6 = param(%t1, 0)\n"
      "0004: int16 %r7 = param(%t1, 1)\n"
      "0005: int32 %r8 = static_cast(%r7)\n"
      "0006: int32 %r9 = mul(%r6, %r8)\n"
      "0007: control %c10 = ret(%c2, %e3, %r9)\n"
      "0008: control %c4 = merge(%c10)\n"
      "0009: void %r5 = exit(%c4)\n",
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
      "0001: control %c2 = control_get(%t1, 0)\n"
      "0002: effect %e3 = effect_get(%t1, 1)\n"
      "0003: int32 %r6 = param(%t1, 0)\n"
      "0004: int32 %r7 = param(%t1, 1)\n"
      "0005: bool %r8 = cmp_le(%r6, %r7)\n"
      "0006: control %c9 = ret(%c2, %e3, %r8)\n"
      "0007: control %c4 = merge(%c9)\n"
      "0008: void %r5 = exit(%c4)\n",
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
      "0001: control %c2 = control_get(%t1, 0)\n"
      "0002: bool %r6 = param(%t1, 0)\n"
      "0003: control %c7 = if(%c2, %r6)\n"
      "0004: control %c9 = if_true(%c7)\n"
      "0005: control %c10 = br(%c9)\n"
      "0006: control %c11 = if_false(%c7)\n"
      "0007: control %c12 = br(%c11)\n"
      "0008: control %c8 = merge(%c10, %c12)\n"
      "0009: effect %e3 = effect_get(%t1, 1)\n"
      "0010: (control, int32) %t14 = phi_operand(%c10, 1)\n"
      "0011: (control, int32) %t15 = phi_operand(%c12, 0)\n"
      "0012: int32 %r13 = phi(%t14, %t15)\n"
      "0013: control %c16 = ret(%c8, %e3, %r13)\n"
      "0014: control %c4 = merge(%c16)\n"
      "0015: void %r5 = exit(%c4)\n",
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
      "0001: control %c2 = control_get(%t1, 0)\n"
      "0002: effect %e3 = effect_get(%t1, 1)\n"
      "0003: control %c6 = ret(%c2, %e3, 123)\n"
      "0004: control %c4 = merge(%c6)\n"
      "0005: void %r5 = exit(%c4)\n",
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
      "0001: control %c2 = control_get(%t1, 0)\n"
      "0002: effect %e3 = effect_get(%t1, 1)\n"
      "0003: int32 %r6 = param(%t1, 0)\n"
      "0004: control %c7 = ret(%c2, %e3, %r6)\n"
      "0005: control %c4 = merge(%c7)\n"
      "0006: void %r5 = exit(%c4)\n",
      Translate("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
