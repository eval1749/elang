// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/analyzer_test.h"
#include "elang/compiler/testing/namespace_builder.h"
#include "elang/compiler/analyze/method_analyzer.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace {

// Install classes and methods for testing.
class MyNamespaceBuilder final : public testing::NamespaceBuilder {
 public:
  explicit MyNamespaceBuilder(NameResolver* name_resolver);
  ~MyNamespaceBuilder() = default;

  void Build();

 private:
  DISALLOW_COPY_AND_ASSIGN(MyNamespaceBuilder);
};

MyNamespaceBuilder::MyNamespaceBuilder(NameResolver* name_resolver)
    : NamespaceBuilder(name_resolver) {
}

void MyNamespaceBuilder::Build() {
  // public class Console {
  //   public static void WriteLine(String string);
  // }
  auto const console_class = session()->ast_factory()->NewClass(
      system_namespace(), Modifiers(Modifier::Public),
      NewKeyword(TokenType::Class), NewName("Console"));
  system_namespace()->AddNamedMember(console_class);

  auto const console_class_body = session()->ast_factory()->NewClassBody(
      system_namespace_body(), console_class);
  session()->global_namespace_body()->AddMember(console_class_body);

  auto const write_line = session()->ast_factory()->NewMethodGroup(
      console_class, NewName("WriteLine"));

  auto const write_line_string = session()->ast_factory()->NewMethod(
      console_class_body, write_line,
      Modifiers(Modifier::Extern, Modifier::Public, Modifier::Static),
      NewTypeReference(TokenType::Void), write_line->name(), {},
      {NewParameter("System.String", "string")}, nullptr);

  auto const write_line_string_object = session()->ast_factory()->NewMethod(
      console_class_body, write_line,
      Modifiers(Modifier::Extern, Modifier::Public, Modifier::Static),
      NewTypeReference(TokenType::Void), write_line->name(), {},
      {NewParameter("System.String", "string"),
       NewParameter("System.Object", "object")},
      nullptr);

  write_line->AddMethod(write_line_string);
  console_class_body->AddMember(write_line_string);
  write_line->AddMethod(write_line_string_object);
  console_class_body->AddMember(write_line_string_object);
  console_class->AddNamedMember(write_line);

  auto const console_ir_class =
      name_resolver()->factory()->NewClass(console_class, {system_object()});

  name_resolver()->DidResolve(console_class, console_ir_class);
}

//////////////////////////////////////////////////////////////////////
//
// MethodAnalyzerTest
//
class MethodAnalyzerTest : public testing::AnalyzerTest {
 protected:
  MethodAnalyzerTest() = default;
  ~MethodAnalyzerTest() override = default;

  // ::testing::Test
  void SetUp() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(MethodAnalyzerTest);
};

// Install methods for testing
void MethodAnalyzerTest::SetUp() {
  MyNamespaceBuilder(name_resolver()).Build();
}

//////////////////////////////////////////////////////////////////////
//
// Method resolution
//
TEST_F(MethodAnalyzerTest, Method) {
  Prepare(
      "using System;"
      "class Sample {"
      "    void Main() { Console.WriteLine(\"Hello world!\"); }"
      "  }");
  EXPECT_EQ("", AnalyzeClass());
  MethodAnalyzer method_analyzer(name_resolver());
  EXPECT_TRUE(method_analyzer.Run());
  EXPECT_EQ("", GetErrors());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
