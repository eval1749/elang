// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/cg/cg_test.h"

#include "elang/compiler/ast/method.h"
#include "elang/compiler/cg/code_generator.h"
#include "elang/compiler/semantics.h"
#include "elang/hir/formatters/text_formatter.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// CodeGeneratorTest
//
class CodeGeneratorTest : public testing::CgTest {
 protected:
  CodeGeneratorTest();
  ~CodeGeneratorTest() override = default;

  const CodeGenerator* code_generator() const { return &code_generator_; }
  CodeGenerator* code_generator() { return &code_generator_; }

  hir::Function* FunctionOf(ast::Method* method) const;

  std::string GetFunction(base::StringPiece name);

 private:
  CodeGenerator code_generator_;

  DISALLOW_COPY_AND_ASSIGN(CodeGeneratorTest);
};

CodeGeneratorTest::CodeGeneratorTest() : code_generator_(session(), factory()) {
}

hir::Function* CodeGeneratorTest::FunctionOf(ast::Method* ast_method) const {
  return code_generator()->FunctionOf(ast_method);
}

std::string CodeGeneratorTest::GetFunction(base::StringPiece name) {
  auto const analyze_result = Analyze();
  if (!analyze_result.empty())
    return analyze_result;
  if (!code_generator()->Run())
    return GetErrors();

  auto const ast_method_group = FindMember(name)->as<ast::MethodGroup>();
  if (!ast_method_group)
    return std::string("No such method group ") + name.as_string();
  auto const ast_method = ast_method_group->methods()[0];
  auto const ir_function = semantics()->ValueOf(ast_method);
  if (!ir_function)
    return std::string("Unbound ") + name.as_string();
  auto const hir_function = FunctionOf(ast_method);
  if (!hir_function)
    return std::string("Not function") + name.as_string();

  std::stringstream ostream;
  hir::TextFormatter formatter(&ostream);
  formatter.FormatFunction(hir_function);
  return ostream.str();
}

//////////////////////////////////////////////////////////////////////
//
// Tests...
//
TEST_F(CodeGeneratorTest, Assignment) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(int x) { x = Bar(); return x; }\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(int32)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  int32 %r2 = entry\n"
      "  int32* %r4 = call `StackAlloc`, void\n"
      "  store %r4, %r2\n"
      "  int32 %r6 = call `Sample.Bar`, void\n"
      "  store %r4, %r6\n"
      "  int32 %r8 = load %r4\n"
      "  ret %r8, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      GetFunction("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Call) {
  Prepare(
      "using System;\n"
      "class Sample {\n"
      "  static void Foo() { Bar(123); }\n"
      "  static void Bar(int x) {}\n"
      "}\n");
  EXPECT_EQ(
      "function1 void(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  call `Sample.Bar`, 123\n"
      "  ret void, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      GetFunction("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Conditional) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(bool x) { var y = x ? Bar() : 38; return y; }\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(bool)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4 block5\n"
      "  bool %b2 = entry\n"
      "  br %b2, block4, block5\n"
      "block4:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  int32 %r4 = call `Sample.Bar`, void\n"
      "  br block3\n"
      "block5:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  br block3\n"
      "block3:\n"
      "  // In: block4 block5\n"
      "  // Out: block2\n"
      "  int32 %r8 = phi block4 %r4, block5 38\n"
      "  ret %r8, block2\n"
      "block2:\n"
      "  // In: block3\n"
      "  // Out:\n"
      "  exit\n",
      GetFunction("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, If) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(bool x) {\n"
      "    if (x) Bar();\n"
      "    if (x)\n"
      "      return 3;\n"
      "    else\n"
      "      Bar();\n"
      "    return 56;\n"
      "  }\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(bool)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block4 block3\n"
      "  bool %b2 = entry\n"
      "  br %b2, block4, block3\n"
      "block4:\n"
      "  // In: block1\n"
      "  // Out: block3\n"
      "  int32 %r4 = call `Sample.Bar`, void\n"
      "  br block3\n"
      "block3:\n"
      "  // In: block4 block1\n"
      "  // Out: block6 block7\n"
      "  br %b2, block6, block7\n"
      "block6:\n"
      "  // In: block3\n"
      "  // Out: block2\n"
      "  ret 3, block2\n"
      "block7:\n"
      "  // In: block3\n"
      "  // Out: block5\n"
      "  int32 %r8 = call `Sample.Bar`, void\n"
      "  br block5\n"
      "block5:\n"
      "  // In: block7\n"
      "  // Out: block2\n"
      "  ret 56, block2\n"
      "block2:\n"
      "  // In: block6 block5\n"
      "  // Out:\n"
      "  exit\n", GetFunction("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Parameter) {
  Prepare(
      "class Sample {\n"
      "  static int Foo(int x) { return x; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(int32)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  int32 %r2 = entry\n"
      "  ret %r2, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      GetFunction("Sample.Foo"));
}

TEST_F(CodeGeneratorTest, Return) {
  Prepare(
      "class Sample {\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  ret 42, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      GetFunction("Sample.Bar"));
}

TEST_F(CodeGeneratorTest, Variable) {
  Prepare(
      "class Sample {\n"
      "  static int Foo() { var x = Bar(); return x; }\n"
      "  static int Bar() { return 42; }\n"
      "}\n");
  EXPECT_EQ(
      "function1 int32(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  int32 %r4 = call `Sample.Bar`, void\n"
      "  ret %r4, block2\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      GetFunction("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
