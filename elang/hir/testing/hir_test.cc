// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <array>
#include <sstream>
#include <string>

#include "elang/base/atomic_string_factory.h"
#include "elang/hir/editor.h"
#include "elang/hir/error_code.h"
#include "elang/hir/error_data.h"
#include "elang/hir/factory.h"
#include "elang/hir/factory_config.h"
#include "elang/hir/formatters/text_formatter.h"
#include "elang/hir/instructions.h"
#include "elang/hir/testing/hir_test.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {
namespace testing {

namespace {

std::string ConvertErrorListToString(const std::vector<ErrorData*> errors) {
  static const char* const mnemonics[] = {
#define V(category, subcategory, name) #category "." #subcategory "." #name,
      FOR_EACH_HIR_ERROR_CODE(V, V)
#undef V
          "Invalid",
  };

  std::stringstream stream;
  for (auto const error : errors) {
    auto const index = std::min(static_cast<size_t>(error->error_code()),
                                arraysize(mnemonics) - 1);
    stream << mnemonics[index] << " " << *error->error_value();
    for (auto detail : error->details())
      stream << " " << *detail;
    stream << std::endl;
  }
  return stream.str();
}

std::unique_ptr<FactoryConfig> NewFactoryConfig(
    AtomicStringFactory* atomic_string_factory) {
  auto config = std::make_unique<FactoryConfig>();
  config->atomic_string_factory = atomic_string_factory;
  config->string_type_name = atomic_string_factory->NewAtomicString(L"String");
  return config;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// HirTest
//
HirTest::HirTest()
    : atomic_string_factory_(new AtomicStringFactory()),
      factory_config_(NewFactoryConfig(atomic_string_factory_.get())),
      factory_(new Factory(*factory_config_)),
      function_(NewFunction(void_type(), void_type())),
      editor_(new Editor(factory(), function_)) {
}

HirTest::~HirTest() {
}

Type* HirTest::bool_type() const {
  return factory()->types()->GetBoolType();
}

BasicBlock* HirTest::entry_block() const {
  return function_->entry_block();
}

BasicBlock* HirTest::exit_block() const {
  return function_->exit_block();
}

Type* HirTest::int32_type() const {
  return types()->GetInt32Type();
}

TypeFactory* HirTest::types() const {
  return factory_->types();
}

Zone* HirTest::zone() const {
  return factory_->zone();
}

Type* HirTest::void_type() const {
  return factory()->void_type();
}

Value* HirTest::void_value() const {
  return factory()->void_value();
}

std::string HirTest::Format(Function* function) {
  std::stringstream stream;
  TextFormatter formatter(&stream);
  formatter.FormatFunction(function);
  return stream.str();
}

std::string HirTest::Format() {
  return Format(function());
}

std::string HirTest::GetErrors() {
  return ConvertErrorListToString(editor_->errors());
}

Value* HirTest::NewBool(bool value) {
  return factory()->NewBoolLiteral(value);
}

Function* HirTest::NewFunction(Type* return_type, Type* parameters_type) {
  return factory()->NewFunction(
      factory()->types()->NewFunctionType(return_type, parameters_type));
}

//      B0---------+    B0 -> B1, B5
//      |          |
//      B1<------+ |    B1 -> B2, B4
//      |        | |
//   +->B2-->B6  | |    B2 -> B3, B6
//   |  |    |   | |
//   +--B3<--+   | |    B3 -> B4, B2
//      |        | |
//      B4<------+ |    B4 -> B1, B5
//      |          |    B6 -> B3
//      B5<--------+
//
//  B0: parent=ENTRY children=[B1, B5]
//  B1: parent=B0    children=[B2, B4]
//  B2: parent=B1    children=[B2, B3]
//  B3: parent=B2    children=[]
//  B4: parent=B1    children=[]
//  B5: parent=B0    children=[EXIT]
//  B6: parent=B2    children=[]
Function* HirTest::NewSampleFunction() {
  auto const function = NewFunction(void_type(), bool_type());
  auto const condition = function->entry_block()->first_instruction();

  Editor editor(factory(), function);

  std::array<BasicBlock*, 7> blocks;
  for (auto& ref : blocks)
    ref = editor.NewBasicBlock(editor.exit_block());

  editor.Edit(editor.entry_block());
  editor.SetBranch(blocks[0]);
  editor.Commit();

  editor.Edit(blocks[0]);
  editor.SetBranch(condition, blocks[1], blocks[5]);
  editor.Commit();

  editor.Edit(blocks[1]);
  editor.SetBranch(condition, blocks[2], blocks[4]);
  editor.Commit();

  editor.Edit(blocks[2]);
  editor.SetBranch(condition, blocks[3], blocks[6]);
  editor.Commit();

  editor.Edit(blocks[3]);
  editor.SetBranch(condition, blocks[2], blocks[4]);
  editor.Commit();

  editor.Edit(blocks[4]);
  editor.SetBranch(condition, blocks[1], blocks[5]);
  editor.Commit();

  editor.Edit(blocks[5]);
  editor.SetReturn(void_value());
  editor.Commit();

  editor.Edit(blocks[6]);
  editor.SetBranch(blocks[3]);
  editor.Commit();

  return function;
}

std::string HirTest::ToString(Type* type) {
  std::stringstream ostream;
  ostream << *type;
  return ostream.str();
}

std::string HirTest::ToString(Value* value) {
  std::stringstream ostream;
  ostream << *value;
  return ostream.str();
}

}  // namespace testing
}  // namespace hir
}  // namespace elang
