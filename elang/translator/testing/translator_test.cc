// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/translator/testing/translator_test.h"

#include "elang/base/atomic_string_factory.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"
#include "elang/optimizer/editor.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/scheduler/schedule.h"
#include "elang/translator/translator.h"

namespace elang {
namespace translator {
namespace testing {

namespace {
ir::Factory* NewHirFactory(api::PassController* pass_controller) {
  ir::FactoryConfig config;
  auto const atomic_string_factory = new AtomicStringFactory();
  config.atomic_string_factory = atomic_string_factory;
  config.string_type_name = atomic_string_factory->NewAtomicString(L"String");
  return new ir::Factory(pass_controller, config);
}
}  // namespace

TranslatorTest::TranslatorTest()
    : ir::FactoryUser(NewHirFactory(this)),
      lir_factory_(new lir::Factory(this)),
      factory_(factory()) {
}

TranslatorTest::~TranslatorTest() {
}

std::string TranslatorTest::Commit(ir::Editor* editor) {
  if (!editor->Validate()) {
    std::stringstream ostream;
    ostream << factory()->errors();
    return ostream.str();
  }
  editor->Commit();
  return "";
}

std::string TranslatorTest::Format(const lir::Function* function) {
  std::stringstream ostream;
  lir::TextFormatter formatter(lir_factory()->literals(), &ostream);
  formatter.FormatFunction(function);
  return ostream.str();
}

std::string TranslatorTest::Translate(const ir::Editor& editor) {
  if (!editor.Validate()) {
    std::stringstream ostream;
    ostream << factory()->errors();
    return ostream.str();
  }
  auto const schedule = factory_->ComputeSchedule(editor.function());
  Translator translator(lir_factory(), schedule.get());
  return TranslatorTest::Format(translator.Run());
}

ir::Function* TranslatorTest::NewFunction(ir::Type* return_type,
                                          ir::Type* parameters_type) {
  return factory()->NewFunction(NewFunctionType(return_type, parameters_type));
}

}  // namespace testing
}  // namespace translator
}  // namespace elang
