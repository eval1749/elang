// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/lir/testing/lir_test.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {
namespace testing {

LirTest::LirTest() : factory_(new Factory()) {
}

Function* LirTest::CreateFunctionEmptySample() {
  auto const function = factory()->NewFunction();
  Editor editor(factory(), function);
  return function;
}

std::string LirTest::FormatFunction(const Function* function) {
  std::stringstream stream;
  TextFormatter formatter(factory(), &stream);
  formatter.FormatFunction(function);
  return stream.str();
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

Value LirTest::NewInt32Value(int32_t data) {
  return factory()->NewInt32Value(data);
}

Value LirTest::NewInt64Value(int64_t data) {
  return factory()->NewInt64Value(data);
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
