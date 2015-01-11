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
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, AliasErrorConflictWithClass) {
  auto const source_code =
      "class R1 {}\n"
      "using R1 = B;\n";
  EXPECT_EQ("Syntax.CompilationUnit.Invalid(12) using\n", Format(source_code))
      << "using directive should come before class";
}

TEST_F(ParserTest, AliasErrorConflictWithNamespace) {
  auto const source_code =
      "namespace R1 {}\n"
      "using R1 = B;\n";
  EXPECT_EQ("Syntax.CompilationUnit.Invalid(16) using\n", Format(source_code))
      << "using directive should come before namespace";
}

TEST_F(ParserTest, AliasErrorDot) {
  EXPECT_EQ("Syntax.Type.Name(13) ;\n", Format("using R1 = A.;\n"));
}

TEST_F(ParserTest, AliasErrorDuplicate) {
  auto const source_code =
      "using R1 = A;\n"
      "using R1 = B;\n";
  EXPECT_EQ("Syntax.UsingDirective.Duplicate(20) R1 R1\n", Format(source_code));
}

TEST_F(ParserTest, AliasErrorReference) {
  EXPECT_EQ("Syntax.Type.Name(11) ;\n", Format("using R1 = ;\n"));
}

//////////////////////////////////////////////////////////////////////
//
// Bracket statement
//
TEST_F(ParserTest, BracketErrorExtra) {
  EXPECT_EQ(
      "Syntax.Bracket.Extra(0) }\n"
      "Syntax.CompilationUnit.Invalid(0) }\n",
      Format("}"));
}

TEST_F(ParserTest, BracketErrorNotClosed) {
  EXPECT_EQ(
      "Syntax.Bracket.NotClosed(12) { )\n"
      "Syntax.Namespace.RightCurryBracket(14) )\n",
      Format("namespace A { )"));
}

TEST_F(ParserTest, BracketErrorNotClosed2) {
  Prepare(
      "class A {\n"
      "  void Run() {\n"
      "    )\n"
      "  }\n"
      "}\n");
  EXPECT_EQ(
      "Syntax.Bracket.NotClosed(8) { )\n"
      "Syntax.Type.Name(29) )\n"
      "Syntax.ClassDecl.RightCurryBracket(29) )\n",
      Format());
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
  EXPECT_EQ(source_code, Format(source_code));
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
// Class
//
TEST_F(ParserTest, ClassAndAlias) {
  auto const source_code =
      "using R = N1.A;\n"
      "namespace N1 {\n"
      "  class R {\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ClassBasic) {
  auto const source_code =
      "class A : C {\n}\n"
      "class B : A {\n}\n"
      "class C {\n}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ClassField) {
  auto const source_code =
      "class A {\n"
      "  int x;\n"
      "  B y = null;\n"
      "  var z = 0;\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ClassErrorConflictToAlias) {
  Prepare("using R = N1.A; class R {}");
  EXPECT_EQ("Syntax.ClassDecl.NameDuplicate(22) R\n", Format());
}

TEST_F(ParserTest, ClassErrorFieldConflict) {
  Prepare("class A { int x() {} bool x; }");
  EXPECT_EQ("Syntax.ClassMember.Conflict(26) x x\n", Format());
}

TEST_F(ParserTest, ClassErrorFieldDuplicate) {
  Prepare("class A { int x; bool x; }");
  EXPECT_EQ("Syntax.ClassMember.Duplicate(22) x x\n", Format());
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
// Conditional expression
//
TEST_F(ParserTest, ConditionalBasic) {
  auto const source_code =
      "class A {\n"
      "  void Method() {\n"
      "    return x ? w : z;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ConditionalErrorColon) {
  auto const source_code =
      "class A {\n"
      "  void Method() {\n"
      "    return x ? w;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.Expression.ConditionalColon(44) ;\n", Format(source_code))
      << "Expect ':' after '?'";
}

TEST_F(ParserTest, ConditionalErrorElse) {
  auto const source_code =
      "class A {\n"
      "  void Method() {\n"
      "    return x ? y :\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.Expression.ConditionalElse(49) }\n", Format(source_code))
      << "Nothing after ':'";
}

TEST_F(ParserTest, ConditionalErrorThen) {
  auto const source_code =
      "class A {\n"
      "  void Method() {\n"
      "    return x ?\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.Expression.ConditionalThen(45) }\n", Format(source_code))
      << "Nothing after '?'";
}

//////////////////////////////////////////////////////////////////////
//
// 'const' statement
//
TEST_F(ParserTest, ConstBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    const var b = 3;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
  EXPECT_EQ(
      "Syntax.Var.NotUsed(25) x\n"
      "Syntax.Var.NotUsed(44) b\n",
      GetWarnings());
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
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
}

//////////////////////////////////////////////////////////////////////
//
// enum
//
TEST_F(ParserTest, EnumBasic) {
  auto const source_code =
      "enum Color {\n"
      "  Red,\n"
      "  Green,\n"
      "  Blue,\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, EnumComma) {
  auto const source_code =
      "enum Color {\n"
      "  Red,\n"
      "  Green,\n"
      "  Blue,\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code)) << "Comma following last member";
}

TEST_F(ParserTest, EnumValue) {
  auto const source_code =
      "enum Color {\n"
      "  Red = 3,\n"
      "  Green = Red + 10,\n"
      "  Blue,\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

//////////////////////////////////////////////////////////////////////
//
// Expression
//
TEST_F(ParserTest, ExpressionArrayBasic) {
  auto const source_code =
      "class A {\n"
      "  void Main(String[] args) {\n"
      "    args[1];\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ExpressionArrayErrorEmpty) {
  Prepare(
      "class A {\n"
      "  void Main(String[] args) {\n"
      "    args[];\n"
      "  }\n"
      "}\n");
  EXPECT_EQ("Syntax.Expression.ArrayAccess(48) ]\n", Format());
}

TEST_F(ParserTest, ExpressionArrayErrorMissingIndex) {
  Prepare(
      "class A {\n"
      "  void Main(String[] args) {\n"
      "    args[1,];\n"
      "  }\n"
      "}\n");
  EXPECT_EQ("Syntax.Expression.ArrayAccess(50) ]\n", Format());
}

TEST_F(ParserTest, ExpressionArrayErrorRightSquareBracket) {
  Prepare(
      "class A {\n"
      "  void Main(String[] args) {\n"
      "    args[1;\n"
      "  }\n"
      "}\n");
  EXPECT_EQ(
      "Syntax.Bracket.NotClosed(47) [ }\n"
      "Syntax.Expression.RightSquareBracket(49) ;\n",
      Format());
}

TEST_F(ParserTest, ExpressionArrayMultiple) {
  auto const source_code =
      "class A {\n"
      "  void Main(String[] args) {\n"
      "    args[1, 2, 3];\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ExpressionCallBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    foo(x);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ExpressionCallErrorMissingArgument) {
  Prepare(
      "class A {\n"
      "  void Run() {\n"
      "    foo(x,);\n"
      "  }\n"
      "}\n");
  EXPECT_EQ("Syntax.Expression.Call(35) )\n", Format());
}

TEST_F(ParserTest, ExpressionErrorSemiColon) {
  Prepare(
      "class A {\n"
      "  void Run() {\n"
      "    foo(x) if (x) bar;\n"  // missing ";" before "if".
      "  }\n"
      "}\n");
  EXPECT_EQ("Syntax.Statement.SemiColon(36) if\n", Format());
}

TEST_F(ParserTest, ExpressionErrorLeftAngleBracket) {
  Prepare(
      "class A {\n"
      "  void Run(int x) {\n"
      "    x<T>;\n"
      "  }\n"
      "}\n");
  EXPECT_EQ(
      "Syntax.Expression.LeftAngleBracket(35) <\n"
      "Syntax.Var.Type(36) T\n"
      "Syntax.Var.SemiColon(37) >\n"
      "Syntax.Type.Name(37) >\n"
      "Syntax.ClassDecl.RightCurryBracket(37) >\n",
      Format());
}

//////////////////////////////////////////////////////////////////////
//
// for-each statement
//
TEST_F(ParserTest, ForEachBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    for (var expr : exprs)\n"
      "      process(expr);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

//////////////////////////////////////////////////////////////////////
//
// 'for' statement
//
TEST_F(ParserTest, ForBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    for (int i = 0; i < 10; ++i)\n"
      "      process(i);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ForConditionOnly) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    for (; condition();)\n"
      "      process(i);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ForErrorSemiColon) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    for (int i = 0; i < 10 ++i)\n"
      "      process(i);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.For.SemiColon(54) i\n", Format(source_code));
}

TEST_F(ParserTest, ForInfiniteLoop) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    for (;;)\n"
      "      process(i);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ForInitOnly) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    for (init();;)\n"
      "      process(i);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ForMultiple) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    for (int i = 0, j = 0; i < 10; ++i, j++)\n"
      "      process(i, j, 20);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ForStepOnly) {
  auto const source_code =
      "class A {\n"
      "  void Run() {\n"
      "    for (;; step())\n"
      "      ;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

//////////////////////////////////////////////////////////////////////
//
// 'if' statement
//
TEST_F(ParserTest, IfBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    if (x)\n"
      "      return x;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
}

//////////////////////////////////////////////////////////////////////
//
// Import
//
TEST_F(ParserTest, ImportBasic) {
  auto const source_code =
      "using System;\n"
      "using System.Console;\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ImportErrorDuplicate) {
  Prepare(
      "using A.B;"
      "using A.B;");
  EXPECT_EQ("Syntax.UsingDirective.Duplicate(16) A.B A.B\n", Format());
}

TEST_F(ParserTest, ImportErrorInvalid) {
  Prepare("using A.B<T>;\n");
  EXPECT_EQ("Syntax.UsingDirective.Import(12) ;\n", Format());
}

//////////////////////////////////////////////////////////////////////
//
// Member Access
//
TEST_F(ParserTest, MemberAccessBasic) {
  auto const source_code =
      "class A {\n"
      "  void F() {\n"
      "    System.Console.WriteLine(123);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, MemberAccessErrorName) {
  Prepare(
      "class A {\n"
      "  void F() {\n"
      "    System.123;\n"  // "." should be followed by name instead of "123".
      "  }\n"
      "}\n");
  EXPECT_EQ("Syntax.MemberAccess.Name(34) 123\n", Format());
}

TEST_F(ParserTest, MemberAccessErrorTypeArgument) {
  Prepare(
      "class A {\n"
      "  void F() {\n"
      "    System.Console<A;\n"
      "  }\n"
      "}\n");
  EXPECT_EQ(
      "Syntax.Bracket.NotClosed(41) < }\n"  // from method F
      "Syntax.MemberAccess.RightAngleBracket(43) ;\n",
      Format());
}

TEST_F(ParserTest, MemberAccessTypeArg) {
  auto const source_code =
      "class A {\n"
      "  void F() {\n"
      "    System.Console<A, int>(123);\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

//////////////////////////////////////////////////////////////////////
//
// Methods
//
TEST_F(ParserTest, MethodBasic) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    return x;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, MethodErrorTypeArg) {
  Prepare(
      "class A {\n"
      "  void Run(B<foo x) {\n"  // Missing right angle bracket.
      "    return 123;\n"
      "  }\n"
      "}\n");
  EXPECT_EQ(
      "Syntax.Bracket.NotClosed(22) < )\n"
      "Syntax.Type.RightAngleBracket(27) x\n",
      Format());
}

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
TEST_F(ParserTest, NamespaceAlias) {
  auto const source_code =
      "namespace A {\n"
      "  using B = N1.N2;\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, NamespaceErrorConflictWithAlias) {
  auto const source_code =
      "using R1 = B;\n"
      "namespace R1 {}\n";
  EXPECT_EQ("Syntax.Namespace.Conflict(24) R1 using\n", Format(source_code));
}

TEST_F(ParserTest, NamespaceErrorConflictWithClass) {
  auto const source_code =
      "class A {}\n"
      "namespace A {}\n";
  EXPECT_EQ("Syntax.Namespace.Conflict(21) A class\n", Format(source_code));
}

TEST_F(ParserTest, NamespaceNestedShortcut) {
  auto const source_code =
      "namespace N1 {\n"
      "  namespace N2 {\n"
      "    class A {\n"
      "    }\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, ReturnExpression) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    return 1;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, UsingVar) {
  auto const source_code =
      "class A {\n"
      "  void Run(int x) {\n"
      "    using (var y = foo) {\n"
      "      foo;\n"
      "    }\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
}

TEST_F(ParserTest, VarErrorAssign) {
  auto const source_code =
      "class A {\n"
      "  void F() {\n"
      "    int x = ;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.Var.Assign(35) ;\n", Format(source_code));
}

TEST_F(ParserTest, VarErrorComma) {
  auto const source_code =
      "class A {\n"
      "  void F() {\n"
      "    int x, ;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.Var.Comma(34) ;\n", Format(source_code));
}

TEST_F(ParserTest, VarErrorDuplicate) {
  auto const source_code =
      "class A {\n"
      "  void F() {\n"
      "    int x, x;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.Var.Duplicate(34) x\n", Format(source_code));
}

TEST_F(ParserTest, VarErrorName) {
  auto const source_code =
      "class A {\n"
      "  void F() {\n"
      "    var ;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.Var.Name(31) ;\n", Format(source_code));
}

TEST_F(ParserTest, VarErrorSemiColon) {
  auto const source_code =
      "class A {\n"
      "  void F() {\n"
      "    int x = 0\n"
      "  }\n"
      "}\n";
  EXPECT_EQ("Syntax.Var.SemiColon(39) }\n", Format(source_code));
}

TEST_F(ParserTest, VarType) {
  auto const source_code =
      "class A {\n"
      "  void F() {\n"
      "    int x = 0, y;\n"
      "    char[] a;\n"
      "    Object[][,,] b;\n"
      "  }\n"
      "}\n";
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
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
  EXPECT_EQ(source_code, Format(source_code));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
