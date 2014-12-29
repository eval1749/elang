// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TESTING_TEST_DRIVER_H_
#define ELANG_COMPILER_TESTING_TEST_DRIVER_H_

#include <memory>
#include <string>

#include "base/strings/string_piece.h"

namespace elang {

namespace compiler {

namespace ast {
class Class;
class NamespaceMember;
}

class CompilationSession;
class CompilationUnit;
class Parser;
class StringSourceCode;

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// TestDriver is a simple harness for testing interactions with compiler.
//
class TestDriver final {
 public:
  explicit TestDriver(base::StringPiece source_text);
  ~TestDriver();

  ast::Class* FindClass(base::StringPiece name);
  ast::NamespaceMember* FindMember(base::StringPiece name);
  std::string GetBaseClasses(base::StringPiece name);
  std::string GetErrors();
  std::string RunNamespaceAnalyzer();
  std::string RunParser();

 private:
  const std::unique_ptr<CompilationSession> session_;
  const std::unique_ptr<StringSourceCode> source_code_;
  const std::unique_ptr<CompilationUnit> compilation_unit_;

  DISALLOW_COPY_AND_ASSIGN(TestDriver);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TESTING_TEST_DRIVER_H_
