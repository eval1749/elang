// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_CG_TEST_H_
#define ELANG_COMPILER_CG_CG_TEST_H_

#include <string>
#include <memory>

#include "elang/base/zone_owner.h"
#include "elang/compiler/testing/analyzer_test.h"

namespace elang {

namespace hir {
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

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// CgTest is a simple harness for testing interactions with compiler.
//
class CgTest : public AnalyzerTest, public ZoneOwner {
 protected:
  CgTest();
  ~CgTest() override;

  CodeGenerator* code_generator() { return code_generator_.get(); }
  hir::Factory* factory() const { return factory_.get(); }
  VariableAnalyzer* variable_analyzer() { return variable_analyzer_.get(); }

  VariableUsages* AnalyzeVariables();
  std::string ConvertToSsa(base::StringPiece name);
  std::string FormatFunction(hir::Function* function);
  hir::Function* FunctionOf(ast::Method* method);
  std::string Generate(base::StringPiece name);
  std::string GetFunction(base::StringPiece name);

 private:
  const std::unique_ptr<hir::FactoryConfig> factory_config_;
  const std::unique_ptr<hir::Factory> factory_;

  std::unique_ptr<VariableAnalyzer> variable_analyzer_;

  // |CodeGenerator| ctor takes |variable_analyzer_|.
  std::unique_ptr<CodeGenerator> code_generator_;

  DISALLOW_COPY_AND_ASSIGN(CgTest);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CG_TEST_H_
