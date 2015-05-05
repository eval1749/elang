// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/cg/testing/cg_test.h"

#include "elang/base/atomic_string_factory.h"
#include "elang/cg/generator.h"
#include "elang/hir/editor.h"
#include "elang/hir/error_data.h"
#include "elang/hir/factory.h"
#include "elang/hir/type_factory.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"

namespace elang {
namespace cg {
namespace testing {

namespace {
hir::Factory* NewHirFactory() {
  hir::FactoryConfig config;
  auto const atomic_string_factory = new AtomicStringFactory();
  config.atomic_string_factory = atomic_string_factory;
  config.string_type_name = atomic_string_factory->NewAtomicString(L"String");
  return new hir::Factory(config);
}
}  // namespace

CgTest::CgTest()
    : hir::FactoryUser(NewHirFactory()),
      lir_factory_(new lir::Factory(this)),
      factory_(factory()),
      function_(NewFunction(void_type(), void_type())) {
}

CgTest::~CgTest() {
}

std::string CgTest::Commit(hir::Editor* editor) {
  if (!editor->Validate(editor->basic_block())) {
    std::stringstream ostream;
    ostream << factory()->errors();
    return ostream.str();
  }
  editor->Commit();
  return "";
}

std::string CgTest::Format(const lir::Function* function) {
  std::stringstream ostream;
  lir::TextFormatter formatter(lir_factory()->literals(), &ostream);
  formatter.FormatFunction(function);
  return ostream.str();
}

std::string CgTest::Generate(hir::Editor* editor) {
  if (!editor->Validate()) {
    std::stringstream ostream;
    ostream << factory()->errors();
    return ostream.str();
  }
  Generator generator(lir_factory(), editor->function());
  return CgTest::Format(generator.Generate());
}

hir::Function* CgTest::NewFunction(hir::Type* return_type,
                                   hir::Type* parameters_type) {
  return factory()->NewFunction(
      types()->NewFunctionType(return_type, parameters_type));
}

}  // namespace testing
}  // namespace cg
}  // namespace elang
