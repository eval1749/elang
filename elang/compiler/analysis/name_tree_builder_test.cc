// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/compiler/testing/analyzer_test.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/analysis_editor.h"
#include "elang/compiler/analysis/name_tree_builder.h"
#include "elang/compiler/ast/nodes.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// NameTreeBuilderTest
//
class NameTreeBuilderTest : public testing::AnalyzerTest {
 protected:
  NameTreeBuilderTest() = default;
  ~NameTreeBuilderTest() override = default;

  std::string BuildNameTree();

 private:
  DISALLOW_COPY_AND_ASSIGN(NameTreeBuilderTest);
};

std::string NameTreeBuilderTest::BuildNameTree() {
  if (!Parse())
    return GetErrors();
  AnalysisEditor editor(session()->analysis());
  NameTreeBuilder(session(), &editor).Run();
  return GetErrors();
}

// Note: MS C# compiler doesn't report error if alias A isn't used.
TEST_F(NameTreeBuilderTest, AliasErrorAmbiguous) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { class A {} }"
      "namespace N3 {"
      "  using A = N1.N2.A;"
      "  class B : A {}"  // A can be N1.N2.A or N3.A.
      "}");
  EXPECT_EQ("NameTree.Alias.Conflict(79) A A\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, AliasConflict) {
  Prepare(
      "namespace N1 { using A = N1; }"
      "namespace N1 { class A {} }");
  EXPECT_EQ("NameTree.Alias.Conflict(21) A A\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, AliasErrorDuplicate) {
  Prepare("namespace N1 { using A = N1; using A = N2; }");
  EXPECT_EQ("Syntax.UsingDirective.Duplicate(35) A A\n", BuildNameTree())
      << "Alias name must be unique in namespace.";
}

TEST_F(NameTreeBuilderTest, ClassBasic) {
  Prepare("class A {} class B{}");
  EXPECT_EQ("", BuildNameTree());
  EXPECT_EQ("#A", ToString(SemanticOf("A")));
  EXPECT_EQ("#B", ToString(SemanticOf("B")));
}

TEST_F(NameTreeBuilderTest, ClassBasicPartial) {
  Prepare("partial class A {}");
  Prepare("partial class A {}");
  EXPECT_EQ("", BuildNameTree());
  EXPECT_EQ("#A", ToString(SemanticOf("A")));
}

TEST_F(NameTreeBuilderTest, ClassErrorConflict) {
  Prepare("class A {}");
  Prepare("namespace A {}");
  EXPECT_EQ("Syntax.Namespace.Conflict(10) A class\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, ClassErrorDuplicate) {
  Prepare("class A {}");
  Prepare("class A {}");
  EXPECT_EQ("Syntax.Class.Duplicate(6) A A\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, ClassErrorDuplicate2) {
  Prepare("namespace System { class Object {} }");
  EXPECT_EQ("Syntax.Class.Duplicate(25) Object Object\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, ConstBasic) {
  Prepare("class A { const int B = 2; }");
  EXPECT_EQ("", BuildNameTree());
  EXPECT_EQ("const ? A.B = ?", ToString(SemanticOf("A.B")));
}

TEST_F(NameTreeBuilderTest, ConstErrorConflict) {
  Prepare("partial class A { const int B = 2; }");
  Prepare("partial class A { int B; }");
  EXPECT_EQ("NameTree.Field.Conflict(22) B B\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, ConstErrorDuplicate) {
  Prepare("partial class A { const int B = 2; }");
  Prepare("partial class A { const int B = 2; }");
  EXPECT_EQ("NameTree.Const.Duplicate(28) B B\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, EnumBasic) {
  Prepare("enum Color { Red, Green, Blue }");
  EXPECT_EQ("", BuildNameTree());
  EXPECT_EQ("#enum Color", ToString(SemanticOf("Color")));
  EXPECT_EQ("Color.Red", ToString(SemanticOf("Color.Red")));
  EXPECT_EQ("Color.Green", ToString(SemanticOf("Color.Green")));
  EXPECT_EQ("Color.Blue", ToString(SemanticOf("Color.Blue")));
}

TEST_F(NameTreeBuilderTest, EnumErrorConflict) {
  Prepare("enum Color { Red }");
  Prepare("class Color {}");
  EXPECT_EQ("Syntax.Class.Conflict(6) Color Color\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, EnumErrorConflict2) {
  Prepare("class A { int Color; enum Color { Red } }");
  EXPECT_EQ("Syntax.Enum.Conflict(26) Color Color\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, EnumErrorDuplicate) {
  Prepare("enum Color { Red }");
  Prepare("enum Color { Blue }");
  EXPECT_EQ("Syntax.Enum.Duplicate(5) Color Color\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, FieldBasic) {
  Prepare("class A { int B = 2; }");
  EXPECT_EQ("", BuildNameTree());
  EXPECT_EQ("A.B", ToString(SemanticOf("A.B")));
}

TEST_F(NameTreeBuilderTest, FieldErrorConflict) {
  Prepare("partial class A { void B() {} }");
  Prepare("partial class A { int B; }");
  EXPECT_EQ("NameTree.Field.Conflict(22) B B\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, FieldErrorDuplicate) {
  Prepare("partial class A { int B = 2; }");
  Prepare("partial class A { int B = 2; }");
  EXPECT_EQ("NameTree.Field.Duplicate(22) B B\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, MethodBasic) {
  Prepare("class A { void B() {} }");
  EXPECT_EQ("", BuildNameTree());
  EXPECT_EQ("#A.B{}", ToString(SemanticOf("A.B")));
}

TEST_F(NameTreeBuilderTest, MethodErrorConflict) {
  Prepare("partial class A { int B; }");
  Prepare("partial class A { void B() {} }");
  EXPECT_EQ("Syntax.ClassMember.Conflict(23) B B\n", BuildNameTree());
}

TEST_F(NameTreeBuilderTest, MethodErrorDuplicate) {
  Prepare("partial class A { void B() {} }");
  Prepare("partial class A { void B() {} }");
  // Since |NameTreeBuilder| doesn't check method signatures, we don't have
  // errors.
  EXPECT_EQ("", BuildNameTree());
}

}  // namespace compiler
}  // namespace elang
