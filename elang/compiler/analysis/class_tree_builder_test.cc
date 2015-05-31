// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/testing/analyzer_test.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/analysis_editor.h"
#include "elang/compiler/analysis/class_tree_builder.h"
#include "elang/compiler/analysis/name_tree_builder.h"
#include "elang/compiler/ast/nodes.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// ClassTreeBuilderTest
//
class ClassTreeBuilderTest : public testing::AnalyzerTest {
 protected:
  ClassTreeBuilderTest() = default;
  ~ClassTreeBuilderTest() override = default;

  std::string BaseClassesOf(base::StringPiece path);
  std::string BuildClassTree();
  sm::Semantic* SemanticOf(base::StringPiece16 path) const;
  sm::Semantic* SemanticOf(base::StringPiece path) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(ClassTreeBuilderTest);
};

std::string ClassTreeBuilderTest::BaseClassesOf(base::StringPiece path) {
  auto const clazz = SemanticOf(path)->as<sm::Class>();
  if (!clazz)
    return "";
  std::stringstream ostream;
  auto separator = "";
  for (auto const base_class : clazz->direct_base_classes()) {
    ostream << separator << ToString(base_class);
    separator = " ";
  }
  return ostream.str();
}

std::string ClassTreeBuilderTest::BuildClassTree() {
  if (!Parse())
    return GetErrors();
  AnalysisEditor analysis_editor(session()->analysis());
  NameTreeBuilder(session(), &analysis_editor).Run();
  if (session()->HasError())
    return GetErrors();
  sm::Editor semantic_editor(session());
  ClassTreeBuilder(session(), &semantic_editor).Run();
  return GetErrors();
}

sm::Semantic* ClassTreeBuilderTest::SemanticOf(base::StringPiece16 path) const {
  sm::Semantic* enclosing = session()->semantic_factory()->global_namespace();
  sm::Semantic* found = static_cast<sm::Semantic*>(nullptr);
  for (size_t pos = 0u; pos < path.length(); ++pos) {
    auto dot_pos = path.find('.', pos);
    if (dot_pos == base::StringPiece::npos)
      dot_pos = path.length();
    auto const name =
        session()->NewAtomicString(path.substr(pos, dot_pos - pos));
    found = enclosing->FindMember(name);
    if (!found)
      return nullptr;
    pos = dot_pos;
    if (pos == path.length())
      break;
    enclosing = found;
    if (!enclosing)
      return nullptr;
  }
  return found;
}

sm::Semantic* ClassTreeBuilderTest::SemanticOf(base::StringPiece path) const {
  return SemanticOf(base::UTF8ToUTF16(path));
}

TEST_F(ClassTreeBuilderTest, AliasBasic) {
  Prepare("namespace N1.N2 { class A {} }");
  Prepare("namespace N3 { using C = N1.N2.A; class B : C {} }");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("N1.N2.A", BaseClassesOf("N3.B"));
}

TEST_F(ClassTreeBuilderTest, AliasConfusing) {
  Prepare(
      "namespace N1 {"
      "  class A {}"
      "  namespace N2 {"
      "    using R1 = A;"  // R1 = N1.A
      "    class A {}"
      "    class B : R1 {}"  // base_class_of(B) = N1.A
      "  }"
      "}");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("N1.A", BaseClassesOf("N1.N2.B"));
}

TEST_F(ClassTreeBuilderTest, AliasErrorNotFound) {
  Prepare(
      "namespace N {"
      "  using R1 = Foo;"  // |Foo| isn't defined anywhere.
      "  class A : R1 {}"
      "}");
  EXPECT_EQ("ClassTree.Name.NotFound(26) Foo\n", BuildClassTree())
      << "Alias references non-existing thing Foo.";
}

// Scope of using alias directive is limited into namespace body.
TEST_F(ClassTreeBuilderTest, AliasErrorScope) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { using R = N1.N2; }"
      "namespace N3 { class B : R.A {} }");  // Error: R unknown
  EXPECT_EQ(
      "ClassTree.Alias.NotUsed(51) R\n"
      "ClassTree.Name.NotFound(88) R\n",
      BuildClassTree());
}

// Note: MS C# compiler doesn't report error for unused alias.
TEST_F(ClassTreeBuilderTest, AliasErrorScopeHide) {
  Prepare(
      "using R = N1.N2;"
      "namespace N1.N2 { class A {} }"
      "namespace N3 {"
      "  class R {}"        // Class R hides alias R in toplevel.
      "  class B : R.A {}"  // Error: R has no member A.
      "}");
  EXPECT_EQ(
      "ClassTree.Alias.NotUsed(6) R\n"
      "ClassTree.Name.NotFound(86) R.A\n",
      BuildClassTree());
}

// Note: MS C# compiler doesn't report error if alias R1 isn't used.
TEST_F(ClassTreeBuilderTest, AliasErrorScopeResolution) {
  Prepare(
      "namespace N1.N2 {}"
      "namespace N3 {"
      "  using R1 = N1;"
      "  using R2 = N1.N2;"
      "  using R3 = R1.N2;"  // Error: R1 is unknown.
      "}");
  EXPECT_EQ(
      "ClassTree.Alias.NotUsed(40) R1\n"
      "ClassTree.Alias.NotUsed(56) R2\n"
      "ClassTree.Alias.NotUsed(75) R3\n"
      "ClassTree.Name.NotFound(80) R1\n",
      BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, AliasExtent) {
  Prepare(
      "using R = N1.N2;"
      "namespace N1.N2 { class A {} }"
      "namespace N3 { class B : R.A {} }"
      "namespace N3 { class C : R.A {} }");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("N1.N2.A", BaseClassesOf("N3.B"));
  EXPECT_EQ("N1.N2.A", BaseClassesOf("N3.C"));
}

// Same as |AliasBasic|, but order of declaration is different.
TEST_F(ClassTreeBuilderTest, AliasLayout) {
  Prepare("namespace N3 { using C = N1.N2.A; class B : C {} }");
  Prepare("namespace N1.N2 { class A {} }");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("N1.N2.A", BaseClassesOf("N3.B"));
}

TEST_F(ClassTreeBuilderTest, AliasToAlias) {
  Prepare(
      "using R1 = A.B;"
      "class A { class B { class C {} } }"
      "namespace N1 {"
      "  using R2 = R1;"
      "  class D : R2.C {}"
      "}");
  ASSERT_EQ("", BuildClassTree());
  EXPECT_EQ("A.B.C", BaseClassesOf("N1.D"));
}

TEST_F(ClassTreeBuilderTest, AliasToAliasDeep) {
  Prepare(
      "using R1 = N1.N2.A.B;"
      "namespace N1 {"
      "  using R2 = R1;"
      "  class D : R2.C {}"
      "  namespace N2 {"
      "     class A { class B { class C {} } }"
      "  }"
      "}");
  ASSERT_EQ("", BuildClassTree());
  EXPECT_EQ("N1.N2.A.B.C", BaseClassesOf("N1.D"));
}

TEST_F(ClassTreeBuilderTest, ClassBasic) {
  Prepare("class A : C {} class B : A {} class C {}");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("C", BaseClassesOf("A"));
  EXPECT_EQ("A", BaseClassesOf("B"));
}

TEST_F(ClassTreeBuilderTest, ClassNested) {
  Prepare("class A { class B {} }");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("System.Object", BaseClassesOf("A"));
  EXPECT_EQ("System.Object", BaseClassesOf("A.B"));
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseNotInterface) {
  Prepare(
      "class A : B, C {}"  // C must be an interface.
      "class B {}"
      "class C {}");
  EXPECT_EQ("ClassTree.BaseClass.NotInterface(13) C\n", BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseStruct) {
  Prepare(
      "class A : S {}"
      "struct S {}");
  EXPECT_EQ("ClassTree.BaseClass.NeitherClassNorInterface(10) S\n",
            BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseClassIsInterface) {
  Prepare(
      "class A : B, C {}"
      "interface B {}"
      "class C {}");
  EXPECT_EQ("ClassTree.BaseClass.NotInterface(13) C\n", BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseClassIsStruct) {
  Prepare(
      "class A : B {}"
      "struct B {}");
  EXPECT_EQ("ClassTree.BaseClass.NeitherClassNorInterface(10) B\n",
            BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseClassIsEnum) {
  Prepare("class A : E {} enum E { E1 }");
  EXPECT_EQ("ClassTree.BaseClass.NeitherClassNorInterface(10) E\n",
            BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseClassIsEnumMember) {
  Prepare("class A : E.E1 {} enum E { E1 }");
  EXPECT_EQ("ClassTree.BaseClass.NeitherClassNorInterface(12) E.E1\n",
            BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseClassIsField) {
  Prepare("class A : B.F {} class B { int F; }");
  EXPECT_EQ("ClassTree.BaseClass.NeitherClassNorInterface(12) B.F\n",
            BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseClassIsMethod) {
  Prepare("class A : B.M {} class B { void M() {} }");
  EXPECT_EQ("ClassTree.BaseClass.NeitherClassNorInterface(12) B.M\n",
            BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorBaseClassIsNamespace) {
  Prepare("namespace N1 { class A : N1 {} }");
  EXPECT_EQ("ClassTree.BaseClass.NeitherClassNorInterface(25) N1\n",
            BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorCircularlyDependency) {
  Prepare(
      "class A : B {}"
      "class B : C {}"
      "class C : A {}");
  EXPECT_EQ(
      "ClassTree.Class.Cycle(6) A B\n"
      "ClassTree.Class.Cycle(20) B C\n"
      "ClassTree.Class.Cycle(34) C A\n",
      BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorCircularlyDependencyNested) {
  Prepare(
      "class A : B.C {}"     // A depends on B and C.
      "class B : A {"        // B depends on A.
      "  public class C {}"  // C depends on B.
      "}");
  EXPECT_EQ(
      "ClassTree.Class.Cycle(6) A C\n"
      "ClassTree.Class.Cycle(22) B A\n"
      "ClassTree.Class.Cycle(44) C B\n",
      BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorDuplicate) {
  Prepare("namespace System { class Int32 {} }");
  // Note: class 'System.Int32' i installed by |ClassTreeBuilderTest|, before
  // paring.
  EXPECT_EQ("Syntax.Class.Duplicate(25) Int32 Int32\n", BuildClassTree());
}

#if 0
TEST_F(ClassTreeBuilderTest, ClassErrorDuplicateWithExtern) {
  Prepare("namespace System { class A {} }");
  EXPECT_TRUE(Parse());
  // Simulate extern module.
  NamespaceBuilder builder(name_resolver());
  builder.NewClass("A", "Object");
  ClassTreeBuilder resolver(name_resolver());
  resolver.Run();
  EXPECT_EQ("ClassTree.Class.Duplicate(25) A A\n", GetErrors());
}
#endif

TEST_F(ClassTreeBuilderTest, ClassErrorNestedDependency) {
  Prepare("class A { class B : A {} }");
  EXPECT_EQ("ClassTree.BaseClass.Containing(20) A B\n", BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ClassErrorSelfReference) {
  Prepare("class A : A {}");
  EXPECT_EQ("ClassTree.BaseClass.Self(6) A A\n", BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ImportBasic) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 {"
      "  using N1.N2;"
      "  class B : A {}"
      "}");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("N1.N2.A", BaseClassesOf("N3.B"));
}

TEST_F(ClassTreeBuilderTest, ImportConfusing) {
  Prepare(
      "namespace N1 { class A {} }"
      "namespace N2 { class A {} }"
      "namespace N3 {"
      "  using N1;"
      "  using N1 = N2;"
      "  class B : N1.A {}"
      "}");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("N2.A", BaseClassesOf("N3.B"));
}

TEST_F(ClassTreeBuilderTest, ImportErrorAmbiguous) {
  Prepare(
      "namespace N1 { class A {} }"
      "namespace N2 { class A {} }"
      "namespace N3 {"
      "  using N1;"
      "  using N2;"
      "  class B : A {}"  // A is ambiguous
      "}");
  EXPECT_EQ("ClassTree.Name.Ambiguous(102) A\n", BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ImportErrorNestNamespace) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 {"
      "  using N1;"
      "  class B : N2.A {}"
      "}");
  EXPECT_EQ("ClassTree.Name.NotFound(67) N2\n", BuildClassTree())
      << "using N1 should not import namespace N1.N2 into N3.";
}

TEST_F(ClassTreeBuilderTest, ImportNotNamespace) {
  Prepare("using System.Object;");
  EXPECT_EQ("ClassTree.Import.NotNamespace(13) System.Object\n",
            BuildClassTree());
}

TEST_F(ClassTreeBuilderTest, ImportNotAmbiguous) {
  Prepare(
      "namespace N1 { class A {} }"
      "namespace N2 { class A {} }"
      "namespace N3 {"
      "  using N1;"
      "  using N2;"
      "  using A = N1.A;"
      "  class B : A {}"  // A means N1.A
      "}");
  EXPECT_EQ("", BuildClassTree());
  EXPECT_EQ("N1.A", BaseClassesOf("N3.B"));
}

}  // namespace compiler
}  // namespace elang
