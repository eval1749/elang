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

 private:
  CodeGenerator code_generator_;

  DISALLOW_COPY_AND_ASSIGN(CodeGeneratorTest);
};

CodeGeneratorTest::CodeGeneratorTest() : code_generator_(session(), factory()) {
}

hir::Function* CodeGeneratorTest::FunctionOf(ast::Method* ast_method) const {
  return code_generator()->FunctionOf(ast_method);
}

//////////////////////////////////////////////////////////////////////
//
// Tests...
//
TEST_F(CodeGeneratorTest, Call) {
  Prepare(
      "using System;\n"
      "class A {\n"
      "  void M1() { M2(123); }\n"
      "  void M2(int x) {}\n"
      "}\n");
  ASSERT_EQ("", Analyze());
  ASSERT_TRUE(code_generator()->Run()) << GetErrors();

  auto const ast_method_group = FindMember("A.M1")->as<ast::MethodGroup>();
  ASSERT_TRUE(ast_method_group) << "No such method group A.M1";
  auto const ast_method = ast_method_group->methods()[0];
  auto const ir_function = semantics()->ValueOf(ast_method);
  ASSERT_TRUE(ir_function);
  auto const hir_function = FunctionOf(ast_method);
  ASSERT_TRUE(hir_function);

  std::stringstream ostream;
  hir::TextFormatter formatter(&ostream);
  formatter.FormatFunction(hir_function);
  EXPECT_EQ(
      "Function void(void)\n"
      "block1:\n"
      "  // In:\n"
      "  // Out: block2\n"
      "  entry\n"
      "  call `A.M2`, void\n"  // TODO(eval1749) s/void/123/
      "  ret void, block2\n"
      "\n"
      "block2:\n"
      "  // In: block1\n"
      "  // Out:\n"
      "  exit\n",
      ostream.str());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
