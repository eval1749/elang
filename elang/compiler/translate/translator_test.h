// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TRANSLATE_TRANSLATOR_TEST_H_
#define ELANG_COMPILER_TRANSLATE_TRANSLATOR_TEST_H_

#include <string>
#include <memory>

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

class CodeGenerator;
class VariableAnalyzer;
class VariableUsages;

namespace cm = compiler;
namespace ir = optimizer;

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// TranslatorTest is a simple harness for testing interactions with compiler.
//
class TranslatorTest : public cm::testing::AnalyzerTest, public ZoneOwner {
 protected:
  TranslatorTest();
  ~TranslatorTest() override;

  ir::Factory* factory() const { return factory_.get(); }

 private:
  const std::unique_ptr<ir::FactoryConfig> factory_config_;
  const std::unique_ptr<ir::Factory> factory_;

  DISALLOW_COPY_AND_ASSIGN(TranslatorTest);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_TRANSLATOR_TEST_H_
