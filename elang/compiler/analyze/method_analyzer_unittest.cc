// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>
#include <vector>

#include "elang/compiler/testing/analyzer_test.h"
#include "elang/compiler/testing/namespace_builder.h"
#include "elang/compiler/analyze/class_analyzer.h"
#include "elang/compiler/analyze/method_analyzer.h"
#include "elang/compiler/analyze/namespace_analyzer.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/visitor.h"
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
// Collector
//
class Collector final : private ast::Visitor {
 public:
  Collector(NameResolver* name_resolver, ast::Method* method);
  Collector() = default;

  NameResolver* name_resolver() const { return name_resolver_; }

  std::string GetCalls() const;

 private:
  // ast::Visitor
  void VisitBlockStatement(ast::BlockStatement* node);
  void VisitCall(ast::Call* node);
  void VisitExpressionStatement(ast::ExpressionStatement* node);
  void VisitVariableReference(ast::VariableReference* node);

  std::vector<ast::Call*> calls_;
  NameResolver* const name_resolver_;
  std::vector<ast::Variable*> variables_;

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

Collector::Collector(NameResolver* name_resolver, ast::Method* method)
    : name_resolver_(name_resolver) {
  auto const body = method->body();
  if (!body)
    return;
  body->Accept(this);
}

std::string Collector::GetCalls() const {
  std::stringstream ostream;
  for (auto const call : calls_) {
    if (auto const method = name_resolver()->ResolveCall(call))
      ostream << *method;
    else
      ostream << "Not resolved: " << *call;
    ostream << std::endl;
  }
  return ostream.str();
}

// ast::Visitor
void Collector::VisitBlockStatement(ast::BlockStatement* node) {
  for (auto const child : node->statements())
    child->Accept(this);
}

void Collector::VisitCall(ast::Call* node) {
  for (auto const child : node->arguments())
    child->Accept(this);
  calls_.push_back(node);
}

void Collector::VisitExpressionStatement(ast::ExpressionStatement* node) {
  node->expression()->Accept(this);
}

void Collector::VisitVariableReference(ast::VariableReference* node) {
  variables_.push_back(node->variable());
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
  ASSERT_EQ("", AnalyzeNamespace());

  ClassAnalyzer class_resolver(name_resolver());
  ASSERT_TRUE(class_resolver.Run()) << GetErrors();

  MethodAnalyzer method_analyzer(name_resolver());
  EXPECT_TRUE(method_analyzer.Run());
  EXPECT_EQ("", GetErrors());
  auto const main_group = FindMember("Sample.Main")->as<ast::MethodGroup>();
  ASSERT_TRUE(main_group);
  auto const method_main = main_group->methods()[0];
  Collector collector(name_resolver(), method_main);
  EXPECT_EQ(
      "(method WriteLine (signature (class Void) ((parameter (class "
      "String)))))\n",
      collector.GetCalls());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
