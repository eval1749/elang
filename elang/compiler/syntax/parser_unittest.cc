// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/compiler_test.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// ParserTest
//
class ParserTest : public testing::CompilerTest {
 protected:
  ParserTest() = default;
  ~ParserTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ParserTest);
};

//////////////////////////////////////////////////////////////////////
//
// Alias
//
TEST_F(ParserTest, AliasBasic) {
  auto const source_code =
      "using R1 = A;\n"
      "using R2 = A.B;\n"
      "using R3 = A.B.C<T>;\n"
      "using R4 = A.B.C<T>.D;\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, AliasErrorDot) {
  auto const source_code = "using R1 = A.;\n";
  Prepare(source_code);
  EXPECT_EQ("Syntax.Type.Name(13) ;\n", Format());
}

TEST_F(ParserTest, AliasErrorDuplicate) {
  auto const source_code =
      "using R1 = A;\n"
      "using R1 = B;\n";
  Prepare(source_code);
  EXPECT_EQ(
      "Syntax.UsingDirective.Duplicate(20) R1 using\n"
      "Syntax.CompilationUnit.Invalid(25) B\n",
      Format());
}

TEST_F(ParserTest, AliasErrorReference) {
  auto const source_code = "using R1 = ;\n";
  Prepare(source_code);
  EXPECT_EQ("Syntax.Type.Name(11) ;\n", Format());
}

//////////////////////////////////////////////////////////////////////
//
// Class
//
TEST_F(ParserTest, ClassAndAlias) {
  auto const source_code =
      "using R = N1.A;\n"
      "namespace N1 {\n"
      "  class R {\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, ClassBasic) {
  auto const source_code =
      "class A : C {\n}\n"
      "class B : A {\n}\n"
      "class C {\n}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, ClassField) {
  auto const source_code =
      "class A {\n"
      "  int x;\n"
      "  B y = null;\n"
      "  var z = 0;\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, ClassErrorConflictToAlias) {
  Prepare("using R = N1.A; class R {}");
  EXPECT_EQ("Syntax.ClassDecl.NameDuplicate(22) R\n", Format());
}

TEST_F(ParserTest, ClassErrorFieldDuplicate) {
  Prepare("class A { int x; bool x; }");
  EXPECT_EQ("Syntax.ClassMember.Duplicate(22) x\n", Format());
}

TEST_F(ParserTest, ClassErrorFieldVar) {
  Prepare("class A { var x; }");
  EXPECT_EQ("Syntax.ClassMember.VarField(14) x\n", Format())
      << "var field must be initialized";
}

//////////////////////////////////////////////////////////////////////
//
// compilation unit
//
TEST_F(ParserTest, CompilationUnitErrorInvalid) {
  Prepare("class A {} using R = A;");
  EXPECT_EQ("Syntax.CompilationUnit.Invalid(11) using\n", Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'break' statement
//
TEST_F(ParserTest, BreakBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    while (x) {\n"
      "      break;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, BreakErrorInvalid) {
  Prepare(
      "class A {"
      "  void Run(int x) {"
      "    break;"
      "  }"
      "}");
  EXPECT_EQ("Syntax.Break.Invalid(40) }\n", Format());
}

TEST_F(ParserTest, BreakErrorSemiColon) {
  Prepare(
      "class A {"
      "  void Run(int x) {"
      "    while (x) {"
      "      break"
      "    }"
      "  }"
      "}");
  EXPECT_EQ("Syntax.Break.SemiColon(58) }\n", Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'const' statement
//
TEST_F(ParserTest, ConstBasic) {
  Prepare("class A { void Run(int x) { const var b = 3; } }");
  EXPECT_EQ(
      "class A {\n"
      "  void Run(int x) {\n"
      "    const var b = 3;\n"
      "  }\n"
      "}\n",
      Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'continue' statement
//
TEST_F(ParserTest, ContinueBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    while (x) {\n"
      "      continue;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, ContinueErrorInvalid) {
  Prepare(
      "class A {"
      "  void Run(int x) {"
      "    continue;"
      "  }"
      "}");
  EXPECT_EQ("Syntax.Continue.Invalid(43) }\n", Format());
}

TEST_F(ParserTest, ContinueErrorSemiColon) {
  Prepare(
      "class A {"
      "  void Run(int x) {"
      "    while (x) {"
      "      continue"
      "    }"
      "  }"
      "}");
  EXPECT_EQ("Syntax.Continue.SemiColon(61) }\n", Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'do' statement
//
TEST_F(ParserTest, DoBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    do {\n"
      "      ;\n"
      "      foo;\n"
      "    } while (x);\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// enum
//
TEST_F(ParserTest, EnumBasic) {
  Prepare("enum Color { Red, Green, Blue }");
  EXPECT_EQ(
      "enum Color {\n"
      "  Red,\n"
      "  Green,\n"
      "  Blue,\n"
      "}\n",
      Format());
}

TEST_F(ParserTest, EnumComma) {
  Prepare("enum Color { Red, Green, Blue, }");
  EXPECT_EQ(
      "enum Color {\n"
      "  Red,\n"
      "  Green,\n"
      "  Blue,\n"
      "}\n",
      Format())
      << "Comma following last member";
}

TEST_F(ParserTest, EnumValue) {
  Prepare("enum Color { Red = 3, Green = Red + 10, Blue }");
  EXPECT_EQ(
      "enum Color {\n"
      "  Red = 3,\n"
      "  Green = Red + 10,\n"
      "  Blue,\n"
      "}\n",
      Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'if' statement
//
TEST_F(ParserTest, IfBasic) {
  Prepare("class A { void Run(int x) { if (x) return x; } }");
  EXPECT_EQ(
      "class A {\n"
      "  void Run(int x) {\n"
      "    if (x)\n"
      "      return x;\n"
      "  }\n"
      "}\n",
      Format());
}

TEST_F(ParserTest, IfBasicElse) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    if (x) {\n"
      "      return x;\n"
      "    } else {\n"
      "      return 3;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// Import
//
TEST_F(ParserTest, ImportBasic) {
  auto const source_code =
      "using System;\n"
      "using System.Console;\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, ImportErrorDuplicate) {
  Prepare("using A.B;"
          "using A.B;");
  EXPECT_EQ("Syntax.UsingDirective.Duplicate(16) A.B A.B\n"
            "Syntax.CompilationUnit.Invalid(19) ;\n",
            Format());
}

TEST_F(ParserTest, ImportErrorInvalid) {
  auto const source_code =
      "using A.B<T>;\n";
  Prepare(source_code);
  EXPECT_EQ("Syntax.UsingDirective.Import(12) ;\n"
            "Syntax.CompilationUnit.Invalid(12) ;\n",
            Format());
}

//////////////////////////////////////////////////////////////////////
//
// Methods
//
TEST_F(ParserTest, MethodBasic) {
  Prepare("class A { void Run(int x) { return x; } }");
  EXPECT_EQ(
      "class A {\n"
      "  void Run(int x) {\n"
      "    return x;\n"
      "  }\n"
      "}\n",
      Format());
}

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
TEST_F(ParserTest, NamespaceAlias) {
  Prepare("namespace A { using B = N1.N2; }");
  EXPECT_EQ(
      "namespace A {\n"
      "  using B = N1.N2;\n"
      "}\n",
      Format());
}

TEST_F(ParserTest, NamespaceBasic) {
  auto const source_code =
      "namespace A {\n"
      "  namespace B {\n"
      "    namespace C {\n"
      "    }\n"
      "  }\n"
      "}\n"
      "namespace D {\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, NamespaceNestedShortcut) {
  auto const source_code =
      "namespace N1 {\n"
      "  namespace N2 {\n"
      "    class A {\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'return' statement
//
TEST_F(ParserTest, ReturnBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    return;\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, ReturnExpression) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    return 1;\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'throw' statement
//
TEST_F(ParserTest, ThrowBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    throw 1;\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, ThrowNoExpression) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    try {\n"
      "      return 1;\n"
      "    } catch (E1) {\n"
      "      throw;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, ThrowInvalid) {
  Prepare("class A { void Run(int x) { throw; } }");
  EXPECT_EQ("Syntax.Throw.Invalid(35) }\n", Format())
      << "We can't omit expression outside 'catch' statement.";
}

//////////////////////////////////////////////////////////////////////
//
// 'try' statement
//
TEST_F(ParserTest, TryBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    try {\n"
      "      return x;\n"
      "    } catch (E y) {\n"
      "      return 3;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, TryCatches) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    try {\n"
      "      return x;\n"
      "    } catch (E1 y) {\n"
      "      return 1;\n"
      "    } catch (E2) {\n"
      "      return 2;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, TryCatcheFinally) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    try {\n"
      "      return x;\n"
      "    } catch (E1 y) {\n"
      "      return 1;\n"
      "    } finally {\n"
      "      return 2;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

TEST_F(ParserTest, TryFinally) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    try {\n"
      "      return x;\n"
      "    } finally {\n"
      "      return 2;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'var' statement
//
TEST_F(ParserTest, VarBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    var a, b = 3;\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'using' statement
//
TEST_F(ParserTest, UsingBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    using (x) {\n"
      "      foo;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'using' statement
//
TEST_F(ParserTest, UsingVar) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    using (var y = foo) {\n"
      "      foo;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'while' statement
//
TEST_F(ParserTest, WhileBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    while (x) {\n"
      "      foo;\n"
      "    }\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

//////////////////////////////////////////////////////////////////////
//
// 'yield' statement
//
TEST_F(ParserTest, YieldBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    yield x;\n"
      "  }\n"
      "}\n";
  Prepare(source_code);
  EXPECT_EQ(source_code, Format());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
