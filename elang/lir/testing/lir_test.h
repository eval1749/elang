// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TESTING_LIR_TEST_H_
#define ELANG_LIR_TESTING_LIR_TEST_H_

#include <string>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/float_types.h"
#include "elang/lir/factory_user.h"
#include "elang/lir/printer_generic.h"
#include "elang/lir/value.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {
class Editor;
class Factory;
class Function;
class Literal;
struct Value;
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// LirTest offers LIR factories.
//
class LirTest : public ::testing::Test, public FactoryUser {
 protected:
  LirTest();
  ~LirTest() override;

  std::string Commit(Editor* editor);
  std::vector<Value> CollectRegisters(const Function* function);
  Function* CreateFunctionEmptySample();
  Function* CreateFunctionSample1();
  Function* CreateFunctionSample2();
  Function* CreateFunctionWithCriticalEdge();

  // Emit instructions to copy parameters to virtual registers and returns
  // list of registers holding parameters.
  std::vector<Value> EmitCopyParameters(Editor* editor, Value type, int count);

  std::string FormatFunction(Editor* editor);
  Literal* GetLiteral(Value value);
  Value NewFloat32Value(float32_t data);
  Value NewFloat64Value(float64_t data);
  Value NewIntValue(ValueSize size, int64_t data);
  Value NewStringValue(base::StringPiece16 data);
  Value NewStringValue(base::StringPiece data);

  // Validates function associated to |editor| and returns validation errors.
  // If function is well-formed, this function returns empty string.
  std::string Validate(Editor* editor);

 private:
  const std::unique_ptr<Factory> factory_;

  DISALLOW_COPY_AND_ASSIGN(LirTest);
};

}  // namespace testing
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TESTING_LIR_TEST_H_
