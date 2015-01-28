// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/cg/cg_test.h"

#include "elang/base/zone_owner.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/cg/cfg_to_ssa_transformer.h"
#include "elang/compiler/cg/code_generator.h"
#include "elang/compiler/cg/variable_analyzer.h"
#include "elang/compiler/semantics.h"
#include "elang/hir/formatters/text_formatter.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaTest
//
class CfgToSsaTest : public testing::CgTest, public ZoneOwner {
 protected:
  CfgToSsaTest();
  ~CfgToSsaTest() override = default;

  CodeGenerator* code_generator() { return &code_generator_; }

  hir::Function* FunctionOf(ast::Method* method);
  std::string CfgToSsaTest::Generate();
  std::string GetFunction(base::StringPiece name);
  VariableUsages* AnalyzeVariables() { return variable_analyzer_.Analyze(); }

 private:
  VariableAnalyzer variable_analyzer_;

  // |CodeGenerator| ctor takes |variable_analyzer_|.
  CodeGenerator code_generator_;

  DISALLOW_COPY_AND_ASSIGN(CfgToSsaTest);
};

CfgToSsaTest::CfgToSsaTest()
    : variable_analyzer_(zone()),
      code_generator_(session(), factory(), &variable_analyzer_) {
}

hir::Function* CfgToSsaTest::FunctionOf(ast::Method* ast_method) {
  return code_generator()->FunctionOf(ast_method);
}

std::string CfgToSsaTest::Generate() {
  auto const analyze_result = Analyze();
  if (!analyze_result.empty())
    return analyze_result;
  if (!code_generator()->Run())
    return GetErrors();
  return "";
}

std::string CfgToSsaTest::GetFunction(base::StringPiece name) {
  auto const generate_result = Generate();
  if (!generate_result.empty())
    return generate_result;

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
TEST_F(CfgToSsaTest, Local) {
  Prepare(
      "class Sample {\n"
      "  static int Foo() {\n"
      "     var x = 1;\n"
      "     x = Bar(x);\n"
      "     x = Baz(x);\n"
      "     return x;\n"
      "  }\n"
      "  static int Bar(int x) { return x; }\n"
      "  static int Baz(int x) { return x; }\n"
      "}\n");
  EXPECT_EQ("foo", GetFunction("Sample.Foo"));
  auto const ast_method =
      FindMember("Sample.Foo")->as<ast::MethodGroup>()->methods()[0];
  auto const function = FunctionOf(ast_method);
  CfgToSsaTransformer pass(factory(), function, AnalyzeVariables());
  EXPECT_EQ("foo", GetFunction("Sample.Foo"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
