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
#include "elang/compiler/semantics.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// Collector
//
class Collector final : private ast::Visitor {
 public:
  Collector(const Semantics* semantics, ast::Method* method);
  Collector() = default;

  const Semantics* semantics() const { return semantics_; }

  std::string GetCalls() const;

 private:
  // ast::Visitor
  void VisitBlockStatement(ast::BlockStatement* node);
  void VisitCall(ast::Call* node);
  void VisitExpressionStatement(ast::ExpressionStatement* node);
  void VisitVarStatement(ast::VarStatement* node);
  void VisitVariableReference(ast::VariableReference* node);

  std::vector<ast::Call*> calls_;
  const Semantics* const semantics_;
  std::vector<ast::Variable*> variables_;

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

Collector::Collector(const Semantics* semantics, ast::Method* method)
    : semantics_(semantics) {
  auto const body = method->body();
  if (!body)
    return;
  body->Accept(this);
}

std::string Collector::GetCalls() const {
  std::stringstream ostream;
  for (auto const call : calls_) {
    if (auto const method = semantics()->ValueOf(call->callee()))
      ostream << *method;
    else
      ostream << "Not resolved: " << *call;
    ostream << std::endl;
  }
  return ostream.str();
}

// ast::Visitor statements
void Collector::VisitBlockStatement(ast::BlockStatement* node) {
  for (auto const child : node->statements())
    child->Accept(this);
}

void Collector::VisitExpressionStatement(ast::ExpressionStatement* node) {
  node->expression()->Accept(this);
}

void Collector::VisitVarStatement(ast::VarStatement* node) {
  for (auto const variable : node->variables()) {
    auto const value = variable->value();
    if (!value)
      continue;
    value->Accept(this);
  }
}

// ast::Visitor expressions
void Collector::VisitCall(ast::Call* node) {
  for (auto const child : node->arguments())
    child->Accept(this);
  calls_.push_back(node);
}

void Collector::VisitVariableReference(ast::VariableReference* node) {
  variables_.push_back(node->variable());
}

//////////////////////////////////////////////////////////////////////
//
// MyNamespaceBuilder
// Installs classes and methods for testing.
//
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

  // Collect calls in method |method_name|.
  std::string GetCalls(base::StringPiece method_name);

  // ::testing::Test
  void SetUp() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(MethodAnalyzerTest);
};

// Install methods for testing
void MethodAnalyzerTest::SetUp() {
  MyNamespaceBuilder(name_resolver()).Build();
}

std::string MethodAnalyzerTest::GetCalls(base::StringPiece method_name) {
  auto const analyze_result = Analyze();
  if (!analyze_result.empty())
    return analyze_result;

  auto const method_group = FindMember(method_name)->as<ast::MethodGroup>();
  if (!method_group)
    return std::string("Not found: ") + method_name.as_string();

  auto const method_main = method_group->methods()[0];
  Collector collector(semantics(), method_main);
  return collector.GetCalls();
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
  EXPECT_EQ(
      "(method WriteLine (signature (class Void) ((parameter (class "
      "String)))))\n",
      GetCalls("Sample.Main"));
}

TEST_F(MethodAnalyzerTest, Method2) {
  Prepare(
      "class Sample {"
      "    static void Foo(char x) {}"
      "    static void Foo(int x) {}"
      "    static void Foo(float32 x) {}"
      "    static void Foo(float64 x) {}"
      "    void Main() { Foo('a'); Foo(123); Foo(12.3); }"
      "  }");
  EXPECT_EQ(
      "(method Foo (signature (class Void) ((parameter (class Char)))))\n"
      "(method Foo (signature (class Void) ((parameter (class Int32)))))\n"
      "(method Foo (signature (class Void) ((parameter (class Float64)))))\n",
      GetCalls("Sample.Main"));
}

TEST_F(MethodAnalyzerTest, Parameter) {
  Prepare(
      "class Sample {"
      "    int Foo(int ival) { return ival; }"
      "    char Foo(char ch) { ch = 'a'; return ch; }"
      "    void Foo(float32 f32) {}"
      "  }");
  EXPECT_EQ("", Analyze());
  auto const foo_group = FindMember("Sample.Foo")->as<ast::MethodGroup>();
  ASSERT_TRUE(foo_group);
  std::stringstream ostream;
  for (auto method : foo_group->methods()) {
    for (auto const parameter : method->parameters()) {
      auto const variable = semantics()->ValueOf(parameter)->as<ir::Variable>();
      if (!variable)
        continue;
      ostream << *parameter->name() << " " << variable->storage() << std::endl;
    }
  }
  // TODO(eval1749) |ch| should be |Local|.
  EXPECT_EQ("ival ReadOnly\nch ReadOnly\nf32 Void\n", ostream.str());
}

TEST_F(MethodAnalyzerTest, ReturnError) {
  Prepare(
      "class Sample {"
      "    int Foo() { return; }"
      "    void Bar() { return 42; }"
      "  }");
  EXPECT_EQ(
      "Method.Return.Void(30) return\n"
      "Method.Return.NotVoid(56) return\n",
      GetCalls("Sample.Foo"));
}

TEST_F(MethodAnalyzerTest, TypeVariable) {
  Prepare(
      "using System;"
      "class Sample {"
      "    static char Foo(char x) { return x; }"
      "    static int Foo(int x) {}"
      "    void Main() { var x = Foo('a'); Foo(x); }"
      "  }");
  EXPECT_EQ(
      "(method Foo (signature (class Char) ((parameter (class Char)))))\n"
      "(method Foo (signature (class Char) ((parameter (class Char)))))\n",
      GetCalls("Sample.Main"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
