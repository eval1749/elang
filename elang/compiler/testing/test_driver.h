// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_testing_compiler_test_h)
#define INCLUDE_elang_compiler_testing_compiler_test_h

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
  std::string GetErrors();
  std::string RunNameResolver();
  std::string RunParser();

 private:
  const std::unique_ptr<CompilationSession> session_;
  const std::unique_ptr<StringSourceCode> source_code_;
  const std::unique_ptr<CompilationUnit> compilation_unit_;

  DISALLOW_COPY_AND_ASSIGN(TestDriver);
};

}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_testing_compiler_test_h)

