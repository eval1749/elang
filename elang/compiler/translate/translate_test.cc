// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/translate/translate_test.h"

#include "base/macros.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/translate/translator.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/factory_config.h"
#include "elang/optimizer/formatters/text_formatter.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {
std::unique_ptr<ir::FactoryConfig> NewFactoryConfig(
    CompilationSession* session) {
  auto config = std::make_unique<ir::FactoryConfig>();
  config->atomic_string_factory = session->atomic_string_factory();
  config->string_type_name = session->NewAtomicString(L"System.String");
  return config;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TranslateTest
//
TranslateTest::TranslateTest()
    : factory_config_(NewFactoryConfig(session())),
      factory_(new ir::Factory(*factory_config_)),
      translator_(new Translator(session(), factory())) {
}

TranslateTest::~TranslateTest() {
}

std::string TranslateTest::FormatFunction(ir::Function* function) {
  std::stringstream ostream;
  ostream << ir::AsReversePostOrder(function);
  return ostream.str();
}

std::string TranslateTest::GetFunction(base::StringPiece name) {
  auto const ast_method_group = FindMember(name)->as<ast::MethodGroup>();
  if (!ast_method_group)
    return std::string("No such method group ") + name.as_string();
  auto const ast_method = ast_method_group->methods()[0];
  auto const sm_function = semantics()->ValueOf(ast_method);
  if (!sm_function)
    return std::string("Unbound ") + name.as_string();
  auto const ir_function = session()->IrFunctionOf(ast_method);
  if (!ir_function)
    return std::string("Not function") + name.as_string();
  return FormatFunction(ir_function);
}

std::string TranslateTest::Translate(base::StringPiece name) {
  auto const analyze_result = Analyze();
  if (!analyze_result.empty())
    return analyze_result;
  if (!translator_->Run())
    return GetErrors();
  return GetFunction(name);
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
