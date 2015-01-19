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
class NameResolver;
class Semantics;
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// AnalyzerTest is a simple harness for testing interactions with compiler.
//
class AnalyzerTest : public CompilerTest {
 protected:
  struct ClassOrString {
    ir::Class* ir_class;
    std::string message;
    explicit ClassOrString(ir::Class* ir_class) : ir_class(ir_class) {}
    ClassOrString(const char* format, base::StringPiece name);
  };

  AnalyzerTest();
  ~AnalyzerTest() override;

  NameResolver* name_resolver() const { return name_resolver_.get(); }
  Semantics* semantics() const;

  std::string AnalyzeClass();
  std::string AnalyzeNamespace();
  std::string GetBaseClasses(base::StringPiece name);
  ClassOrString GetClass(base::StringPiece name);
  std::string GetDirectBaseClasses(base::StringPiece name);
  std::string GetMethodGroup(base::StringPiece name);
  std::string MakeClassListString(const std::vector<ir::Class*>& classes);
  std::string MakeClassListString(const ZoneVector<ir::Class*>& classes);

 private:
  std::unique_ptr<NameResolver> name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(AnalyzerTest);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TESTING_ANALYZER_TEST_H_
