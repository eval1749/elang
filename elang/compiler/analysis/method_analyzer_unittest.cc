// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "elang/compiler/testing/analyzer_test.h"
#include "elang/compiler/testing/namespace_builder.h"
#include "elang/compiler/analysis/class_analyzer.h"
#include "elang/compiler/analysis/method_analyzer.h"
#include "elang/compiler/analysis/namespace_analyzer.h"
#include "elang/compiler/analysis/name_resolver.h"
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

  const std::vector<ast::NamedNode*> variables() const { return variables_; }

  std::string GetCalls() const;

 private:
  const Semantics* semantics() const { return semantics_; }

  // ast::Visitor expressions
  void VisitCall(ast::Call* node);

  // ast::Visitor statements
  void VisitBlockStatement(ast::BlockStatement* node);
  void VisitExpressionStatement(ast::ExpressionStatement* node);
  void VisitForEachStatement(ast::ForEachStatement* node);
  void VisitVarStatement(ast::VarStatement* node);

  std::vector<ast::Call*> calls_;
  const Semantics* const semantics_;
  std::vector<ast::NamedNode*> variables_;

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

Collector::Collector(const Semantics* semantics, ast::Method* method)
    : semantics_(semantics) {
  for (auto const parameter : method->parameters())
    variables_.push_back(parameter);
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

void Collector::VisitForEachStatement(ast::ForEachStatement* node) {
  variables_.push_back(node->variable());
  node->enumerable()->Accept(this);
  node->statement()->Accept(this);
}

void Collector::VisitVarStatement(ast::VarStatement* node) {
  for (auto const variable : node->variables()) {
    variables_.push_back(variable);
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

  // Collect all semantics
  std::string QuerySemantics(TokenType token_type);

  // Collect calls in method |method_name|.
  std::string GetCalls(base::StringPiece method_name);

  // Collect variables used in method |method_name|.
  std::string VariablesOf(base::StringPiece method_name);

  // ::testing::Test
  void SetUp() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(MethodAnalyzerTest);
};

std::string MethodAnalyzerTest::QuerySemantics(TokenType token_type) {
  typedef std::pair<ast::Node*, ir::Node*> KeyValue;
  std::vector<KeyValue> key_values;
  for (auto const key_value : semantics()->all()) {
    if (!key_value.first->token()->location().start_offset())
      continue;
    if (key_value.first->token() != token_type)
      continue;
    key_values.push_back(key_value);
  }
  std::sort(key_values.begin(), key_values.end(),
            [](const KeyValue& a, const KeyValue& b) {
              return a.first->token()->location().start_offset() <
                     b.first->token()->location().start_offset();
            });
  std::stringstream ostream;
  for (auto const key_value : key_values)
    ostream << *key_value.second << std::endl;
  return ostream.str();
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

std::string MethodAnalyzerTest::VariablesOf(base::StringPiece method_name) {
  auto const analyze_result = Analyze();
  if (!analyze_result.empty())
    return analyze_result;

  auto const method_group = FindMember(method_name)->as<ast::MethodGroup>();
  if (!method_group)
    return std::string("Not found: ") + method_name.as_string();

  auto const method_main = method_group->methods()[0];
  Collector collector(semantics(), method_main);
  std::stringstream ostream;
  for (auto const variable : collector.variables())
    ostream << *semantics()->ValueOf(variable) << std::endl;
  return ostream.str();
}

// Install methods for testing
void MethodAnalyzerTest::SetUp() {
  MyNamespaceBuilder(name_resolver()).Build();
}

//////////////////////////////////////////////////////////////////////
//
// Test cases
//

// Array access
TEST_F(MethodAnalyzerTest, ArrayAccess) {
  Prepare(
      "using System;"
      "class Sample {"
      "  static void Main(String[] args) {"
      "    Console.WriteLine(args[1]);"
      "  }"
      "}");
  ASSERT_EQ("", Analyze());
  EXPECT_EQ("System.String[]\n", QuerySemantics(TokenType::LeftSquareBracket));
}

TEST_F(MethodAnalyzerTest, ArrayAccessErrorArray) {
  Prepare(
      "using System;"
      "class Sample {"
      "  static void Main(int args) {"
      "    Console.WriteLine(args[1]);"
      "  }"
      "}");
  ASSERT_EQ("TypeResolver.ArrayAccess.Array(79) args\n", Analyze());
}

TEST_F(MethodAnalyzerTest, ArrayAccessErrorIndex) {
  Prepare(
      "using System;"
      "class Sample {"
      "  static void Main(String[] args) {"
      "    Console.WriteLine(args[\"foo\"]);"
      "  }"
      "}");
  ASSERT_EQ("TypeResolver.ArrayAccess.Index(89) \"foo\"\n", Analyze());
}

TEST_F(MethodAnalyzerTest, ArrayAccessErrorRank) {
  Prepare(
      "using System;"
      "class Sample {"
      "  static void Main(int[] args) {"
      "    Console.WriteLine(args[1, 2]);"
      "  }"
      "}");
  ASSERT_EQ("TypeResolver.ArrayAccess.Rank(85) [\n", Analyze());
}

// Binary operations
TEST_F(MethodAnalyzerTest, BinaryOperationArithmeticFloat64) {
  Prepare(
      "class Sample {"
      "  void Foo(float64 f64, float32 f32,"
      "            int8 i8, int16 i16, int32 i32, int64 i64,"
      "            uint8 u8, uint16 u16, uint32 u32, uint64 u64) {"
      "    var f64_f32 = f64 + f32;"
      "    var f64_f64 = f64 + f64;"
      ""
      "    var f64_i8 = f64 + i8;"
      "    var f64_i16 = f64 + i16;"
      "    var f64_i32 = f64 + i32;"
      "    var f64_i64 = f64 + i64;"
      ""
      "    var f64_u8 = f64 + u8;"
      "    var f64_u16 = f64 + u16;"
      "    var f64_u32 = f64 + u32;"
      "    var f64_u64 = f64 + u64;"
      "  }"
      "}");
  ASSERT_EQ("", Analyze());
  EXPECT_EQ(
      "System.Float64\n"
      "System.Float64\n"
      "System.Float64\n"
      "System.Float64\n"
      "System.Float64\n"
      "System.Float64\n"
      "System.Float64\n"
      "System.Float64\n"
      "System.Float64\n"
      "System.Float64\n",
      QuerySemantics(TokenType::Add));
}

TEST_F(MethodAnalyzerTest, BinaryOperationArithmeticFloat32) {
  Prepare(
      "class Sample {"
      "  void Foo(float64 f64, float32 f32,"
      "            int8 i8, int16 i16, int32 i32, int64 i64,"
      "            uint8 u8, uint16 u16, uint32 u32, uint64 u64) {"
      "    var f32_f32 = f32 + f32;"
      "    var f32_f64 = f32 + f64;"
      ""
      "    var f32_i8 = f32 + i8;"
      "    var f32_i16 = f32 + i16;"
      "    var f32_i32 = f32 + i32;"
      "    var f32_i64 = f32 + i64;"
      ""
      "    var f32_u8 = f32 + u8;"
      "    var f32_u16 = f32 + u16;"
      "    var f32_u32 = f32 + u32;"
      "    var f32_u64 = f32 + u64;"
      "  }"
      "}");
  ASSERT_EQ("", Analyze());
  EXPECT_EQ(
      "System.Float32\n"
      "System.Float64\n"
      "System.Float32\n"
      "System.Float32\n"
      "System.Float32\n"
      "System.Float32\n"
      "System.Float32\n"
      "System.Float32\n"
      "System.Float32\n"
      "System.Float32\n",
      QuerySemantics(TokenType::Add));
}

// Conditional expression
TEST_F(MethodAnalyzerTest, Conditional) {
  Prepare(
      "class Sample {"
      "    void Main() { Foo(Cond() ? 12 : 34); }"
      "    bool Cond() { return true; }"
      "    int Foo(int x) { return x; }"
      "  }");
  EXPECT_EQ("", Analyze());
}

TEST_F(MethodAnalyzerTest, ConditionalErrorBool) {
  Prepare(
      "class Sample {"
      "    void Main() { Foo(Cond() ? 12 : 34); }"
      "    int Cond() { return 12; }"
      "    int Foo(int x) { return x; }"
      "  }");
  EXPECT_EQ("TypeResolver.Expression.NotBool(36) Cond\n", Analyze());
}

TEST_F(MethodAnalyzerTest, ConditionalErrorResult) {
  Prepare(
      "class Sample {"
      "    void Main() { Cond() ? 12 : 34.0; }"
      "    bool Cond() { return true; }"
      "  }");
  EXPECT_EQ("TypeResolver.Conditional.NotMatch(41) 12 34\n", Analyze());
}

// 'do' statement
TEST_F(MethodAnalyzerTest, Do) {
  Prepare(
      "class Sample {"
      "    void Main() { do { Foo(12); } while (Cond()); }"
      "    bool Cond() { return true; }"
      "    int Foo(int x) { return x; }"
      "  }");
  EXPECT_EQ("", Analyze());
}

TEST_F(MethodAnalyzerTest, DoErrorCondition) {
  Prepare(
      "class Sample {"
      "    void Main() { do { Foo(0); } while (Foo(1)); }"
      "    abstract Sample Foo(int x);"
      "  }");
  EXPECT_EQ("TypeResolver.Expression.NotBool(54) Foo\n", Analyze());
}

// 'for' statement
TEST_F(MethodAnalyzerTest, For) {
  Prepare(
      "class Sample {"
      "    void Main() { for (Foo(3); Cond(); Foo(4)) { Foo(12); } }"
      "    bool Cond() { return true; }"
      "    int Foo(int x) { return x; }"
      "  }");
  EXPECT_EQ("", Analyze());
}

TEST_F(MethodAnalyzerTest, ForErrorCondition) {
  Prepare(
      "class Sample {"
      "    void Main() { for (;Foo(1);) { Foo(0); } }"
      "    abstract Sample Foo(int x);"
      "  }");
  EXPECT_EQ("TypeResolver.Expression.NotBool(38) Foo\n", Analyze());
}

// for each statement
TEST_F(MethodAnalyzerTest, ForEach) {
  Prepare(
      "using System;"
      "class Sample {"
      "  static void Main(String[] args) {"
      "    for (var arg : args)"
      "      Console.WriteLine(arg);"
      "  }"
      "}");
  EXPECT_EQ(
      "ReadOnly System.String[] args\n"
      "ReadOnly System.String arg\n",
      VariablesOf("Sample.Main"));
}

TEST_F(MethodAnalyzerTest, ForEachError) {
  Prepare(
      "using System;"
      "class Sample {"
      "  static void Main(String[] args) {"
      "    for (int arg : args)"
      "      Console.WriteLine(arg);"
      "  }"
      "}");
  EXPECT_EQ("TypeResolver.ForEach.ElementType(75) arg\n",
            VariablesOf("Sample.Main"));
}

// 'if' statement
TEST_F(MethodAnalyzerTest, If) {
  Prepare(
      "class Sample {"
      "    void Main() { if (Cond()) Foo(12); }"
      "    void Other() { if (Cond()) Foo(12); else Foo(34); }"
      "    bool Cond() { return true; }"
      "    int Foo(int x) { return x; }"
      "  }");
  EXPECT_EQ("", Analyze());
}

TEST_F(MethodAnalyzerTest, IfErrorCondition) {
  Prepare(
      "class Sample {"
      "    void Main() { if (Foo(0)) Foo(12); else Foo(34); }"
      "    abstract Sample Foo(int x);"
      "  }");
  EXPECT_EQ("TypeResolver.Expression.NotBool(36) Foo\n", Analyze());
}

// Method resolution
TEST_F(MethodAnalyzerTest, Method) {
  Prepare(
      "using System;"
      "class Sample {"
      "    void Main() { Console.WriteLine(\"Hello world!\"); }"
      "  }");
  EXPECT_EQ("System.Void System.Console.WriteLine(System.String string)\n",
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
      "System.Void Sample.Foo(System.Char x)\n"
      "System.Void Sample.Foo(System.Int32 x)\n"
      "System.Void Sample.Foo(System.Float64 x)\n",
      GetCalls("Sample.Main"));
}

TEST_F(MethodAnalyzerTest, Parameter) {
  Prepare(
      "class Sample {"
      "    int Foo(int ival) { return ival; }"          // ReadOnly
      "    char Foo(char ch) { ch = 'a'; return ch; }"  // Local
      "    void Foo(float32 f32) {}"                    // Void == no references
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
  EXPECT_EQ("ival ReadOnly\nch Local\nf32 Void\n", ostream.str());
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
      "System.Char Sample.Foo(System.Char x)\n"
      "System.Char Sample.Foo(System.Char x)\n",
      GetCalls("Sample.Main"));
}

// 'while' statement
TEST_F(MethodAnalyzerTest, While) {
  Prepare(
      "class Sample {"
      "    void Main() {  while (Cond()) { Foo(12); } }"
      "    bool Cond() { return true; }"
      "    int Foo(int x) { return x; }"
      "  }");
  EXPECT_EQ("", Analyze());
}

TEST_F(MethodAnalyzerTest, WhileErrorCondition) {
  Prepare(
      "class Sample {"
      "    void Main() { while (Foo(1)) { Foo(0); } }"
      "    abstract Sample Foo(int x);"
      "  }");
  EXPECT_EQ("TypeResolver.Expression.NotBool(39) Foo\n", Analyze());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
