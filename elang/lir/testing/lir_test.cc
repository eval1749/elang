// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/lir/testing/lir_test.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/lir/error_data.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"
#include "elang/lir/literals.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {
namespace testing {

LirTest::LirTest() : factory_(new Factory()) {
}

std::string LirTest::Commit(Editor* editor) {
  if (editor->Validate(editor->basic_block())) {
    editor->Commit();
    return "";
  }
  std::stringstream ostream;
  ostream << editor->errors();
  return ostream.str();
}

Function* LirTest::CreateFunctionEmptySample() {
  auto const function = factory()->NewFunction();
  Editor editor(factory(), function);
  return function;
}

Function* LirTest::CreateFunctionSample1() {
  auto const function = factory()->NewFunction();
  Editor editor(factory(), function);
  auto const entry_block = function->entry_block();
  {
    Editor::ScopedEdit scope(&editor);
    editor.Edit(entry_block);
    auto const call = factory()->NewCallInstruction(NewStringValue("Foo"));
    editor.InsertBefore(call, entry_block->last_instruction());
  }
  return function;
}

std::string LirTest::FormatFunction(Editor* editor) {
  std::stringstream ostream;
  if (!editor->Validate()) {
    ostream << editor->errors();
  } else {
    TextFormatter formatter(factory()->literals(), &ostream);
    formatter.FormatFunction(editor->function());
  }
  return ostream.str();
}

Literal* LirTest::GetLiteral(Value value) {
  return factory()->GetLiteral(value);
}

Value LirTest::NewFloat32Value(float32_t data) {
  return factory()->NewFloat32Value(data);
}

Value LirTest::NewFloat64Value(float64_t data) {
  return factory()->NewFloat64Value(data);
}

Value LirTest::NewIntValue(ValueSize size, int64_t data) {
  return factory()->NewIntValue(size, data);
}

Value LirTest::NewStringValue(base::StringPiece16 data) {
  return factory()->NewStringValue(data);
}
Value LirTest::NewStringValue(base::StringPiece data) {
  return factory()->NewStringValue(base::UTF8ToUTF16(data));
}

}  // namespace testing
}  // namespace lir
}  // namespace elang
