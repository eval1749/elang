// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TESTING_ANALYZER_TEST_H_
#define ELANG_COMPILER_TESTING_ANALYZER_TEST_H_

#include <string>
#include <vector>

#include "elang/compiler/testing/compiler_test.h"

namespace elang {
namespace compiler {
class Analysis;
class NameResolver;
namespace sm {
class Semantic;
}
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// AnalyzerTest is a simple harness for testing interactions with compiler.
//
class AnalyzerTest : public CompilerTest {
 protected:
  AnalyzerTest();
  ~AnalyzerTest() override;

  NameResolver* name_resolver() const { return name_resolver_.get(); }
  Analysis* analysis() const;

  std::string Analyze();
  std::string AnalyzeClass();
  std::string AnalyzeNamespace();
  std::string GetMethodGroup(base::StringPiece name);
  std::string MakeClassListString(const std::vector<sm::Class*>& classes);
  std::string MakeClassListString(const ZoneVector<sm::Class*>& classes);
  sm::Semantic* SemanticOf(ast::Node* node);
  std::string ToString(sm::Semantic* semantic);

 private:
  std::unique_ptr<NameResolver> name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(AnalyzerTest);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TESTING_ANALYZER_TEST_H_
