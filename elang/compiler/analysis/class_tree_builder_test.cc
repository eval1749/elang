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
  EXPECT_EQ("NameResolution.Name.NotFound(26) Foo\n", BuildClassTree())
      << "Alias references non-existing thing Foo.";
}

// Scope of using alias directive is limited into namespace body.
TEST_F(ClassTreeBuilderTest, AliasErrorScope) {
  Prepare(
      "namespace N1.N2 { class A {} }"
      "namespace N3 { using R = N1.N2; }"
      "namespace N3 { class B : R.A {} }");  // Error: R unknown
  EXPECT_EQ(
      "NameResolution.Alias.NotUsed(51) R\n"
      "NameResolution.Name.NotFound(88) R\n",
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
      "NameResolution.Alias.NotUsed(6) R\n"
      "NameResolution.Name.NotFound(86) R.A\n",
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
      "NameResolution.Alias.NotUsed(40) R1\n"
      "NameResolution.Alias.NotUsed(56) R2\n"
      "NameResolution.Alias.NotUsed(75) R3\n"
      "NameResolution.Name.NotFound(80) R1\n",
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

}  // namespace compiler
}  // namespace elang
