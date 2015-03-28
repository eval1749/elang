// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TRANSLATE_TRANSLATE_TEST_H_
#define ELANG_COMPILER_TRANSLATE_TRANSLATE_TEST_H_

#include <string>
#include <memory>

#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/testing/analyzer_test.h"

namespace elang {

namespace optimizer {
class Factory;
struct FactoryConfig;
class Function;
}

namespace compiler {

namespace ast {
class Method;
}

class Translator;

namespace ir = optimizer;

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// TranslateTest is a simple harness for testing interactions with compiler.
//
class TranslateTest : public testing::AnalyzerTest, public ZoneOwner {
 protected:
  TranslateTest();
  ~TranslateTest() override;

  ir::Factory* factory() const { return factory_.get(); }

  std::string Translate(base::StringPiece function_name);

 private:
  std::string FormatFunction(ir::Function* function);
  std::string GetFunction(base::StringPiece name);

  const std::unique_ptr<ir::FactoryConfig> factory_config_;
  const std::unique_ptr<ir::Factory> factory_;
  const std::unique_ptr<Translator> translator_;

  DISALLOW_COPY_AND_ASSIGN(TranslateTest);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_TRANSLATE_TEST_H_
