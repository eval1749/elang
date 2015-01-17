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
  auto const console_class_body = NewClass("Console", "Object");
  auto const console_class = console_class_body->owner();

  auto const write_line = session()->ast_factory()->NewMethodGroup(
      console_class, NewName("WriteLine"));

  auto const write_line_string = session()->ast_factory()->NewMethod(
      console_class_body, write_line,
      Modifiers(Modifier::Extern, Modifier::Public, Modifier::Static),
      NewTypeReference(TokenType::Void), write_line->name(), {});
  write_line_string->SetParameters({
      NewParameter(write_line_string, 0, "System.String", "string"),
  });

  auto const write_line_string_object = session()->ast_factory()->NewMethod(
      console_class_body, write_line,
      Modifiers(Modifier::Extern, Modifier::Public, Modifier::Static),
      NewTypeReference(TokenType::Void), write_line->name(), {});
  write_line_string_object->SetParameters({
      NewParameter(write_line_string_object, 0, "System.String", "string"),
      NewParameter(write_line_string_object, 1, "System.Object", "object"),
  });

  write_line->AddMethod(write_line_string);
  console_class_body->AddMember(write_line_string);
  write_line->AddMethod(write_line_string_object);
  console_class_body->AddMember(write_line_string_object);
  console_class->AddNamedMember(write_line);
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
