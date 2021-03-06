// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/cg/cg_test.h"

#include "base/macros.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/cg/code_generator.h"
#include "elang/compiler/cg/cfg_to_ssa_converter.h"
#include "elang/compiler/cg/variable_analyzer.h"
#include "elang/compiler/compilation_session.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/factory_config.h"
#include "elang/hir/formatters/text_formatter.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {
std::unique_ptr<hir::FactoryConfig> NewFactoryConfig(
    CompilationSession* session) {
  auto config = std::make_unique<hir::FactoryConfig>();
  config->atomic_string_factory = session->atomic_string_factory();
  config->string_type_name = session->NewAtomicString(L"System.String");
  return config;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CgTest
//
CgTest::CgTest()
    : factory_config_(NewFactoryConfig(session())),
      factory_(new hir::Factory(*factory_config_)),
      variable_analyzer_(new VariableAnalyzer(zone())),
      code_generator_(
          new CodeGenerator(session(), factory(), variable_analyzer())) {
}

CgTest::~CgTest() {
}

VariableUsages* CgTest::AnalyzeVariables() {
  return variable_analyzer_->Analyze();
}

std::string CgTest::ConvertToSsa(base::StringPiece name) {
  auto const ast_method = FindMember(name)->as<ast::Method>();
  auto const function = FunctionOf(ast_method);
  if (!function)
    return GetErrors();
  hir::Editor editor(factory(), function);
  CfgToSsaConverter pass(&editor, AnalyzeVariables());
  pass.Run();
  return GetFunction(name);
}

std::string CgTest::FormatFunction(hir::Function* function) {
  std::ostringstream ostream;
  hir::TextFormatter formatter(&ostream);
  formatter.FormatFunction(function);
  return ostream.str();
}

hir::Function* CgTest::FunctionOf(ast::Method* ast_method) {
  return session()->FunctionOf(ast_method);
}

std::string CgTest::Generate(base::StringPiece name) {
  auto const analyze_result = Analyze();
  if (!analyze_result.empty())
    return analyze_result;
  code_generator()->Run();
  if (session()->HasError())
    return GetErrors();
  return GetFunction(name);
}

std::string CgTest::GetFunction(base::StringPiece name) {
  auto const ast_method = FindMember(name)->as<ast::Method>();
  if (!ast_method)
    return std::string("No such method ") + name.as_string();
  auto const ir_function = analysis()->SemanticOf(ast_method);
  if (!ir_function)
    return std::string("Unbound ") + name.as_string();
  auto const hir_function = FunctionOf(ast_method);
  if (!hir_function)
    return std::string("Not function") + name.as_string();
  return FormatFunction(hir_function);
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
