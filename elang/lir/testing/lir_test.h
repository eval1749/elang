// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TESTING_LIR_TEST_H_
#define ELANG_LIR_TESTING_LIR_TEST_H_

#include <string>
#include <memory>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/float_types.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {
class Factory;
class Function;
class Literal;
struct Value;
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// LirTest offers LIR factories.
//
class LirTest : public ::testing::Test {
 protected:
  LirTest();
  ~LirTest() override = default;

  Factory* factory() { return factory_.get(); }

  Function* CreateFunctionEmptySample();
  std::string FormatFunction(const Function* function);
  Literal* GetLiteral(Value value);
  Value NewFloat32Value(float32_t data);
  Value NewFloat64Value(float64_t data);
  Value NewInt32Value(int32_t data);
  Value NewInt64Value(int64_t data);
  Value NewStringValue(base::StringPiece16 data);
  Value NewStringValue(base::StringPiece data);

 private:
  const std::unique_ptr<Factory> factory_;

  DISALLOW_COPY_AND_ASSIGN(LirTest);
};

}  // namespace testing
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TESTING_LIR_TEST_H_
