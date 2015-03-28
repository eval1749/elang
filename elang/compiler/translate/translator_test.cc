// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/compiler/translate/translator_test.h"

#include "base/macros.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/factory_config.h"
#include "elang/optimizer/formatters/text_formatter.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {
std::unique_ptr<ir::FactoryConfig> NewFactoryConfig(
    cm::CompilationSession* session) {
  auto config = std::make_unique<ir::FactoryConfig>();
  config->atomic_string_factory = session->atomic_string_factory();
  config->string_type_name = session->NewAtomicString(L"System.String");
  return config;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TranslatorTest
//
TranslatorTest::TranslatorTest()
    : factory_config_(NewFactoryConfig(session())),
      factory_(new ir::Factory(*factory_config_)) {
}

TranslatorTest::~TranslatorTest() {
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
