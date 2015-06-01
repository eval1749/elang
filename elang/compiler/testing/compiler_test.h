// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TESTING_COMPILER_TEST_H_
#define ELANG_COMPILER_TESTING_COMPILER_TEST_H_

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/base/zone_vector.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {

namespace ast {
class Class;
class Node;
class NamedNode;
}

namespace sm {
class Class;
}

class CompilationSession;
class SourceCode;
class StringSourceCode;
enum class TokenType;

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// CompilerTest is a simple harness for testing interactions with compiler.
//
class CompilerTest : public ::testing::Test {
 protected:
  CompilerTest();
  ~CompilerTest() override;

  CompilationSession* session() const { return session_.get(); }

  // Since we don't want to include "string_source_code.h" in "compiler_test.h",
  // we implement |source_code()| function in ".cc".
  SourceCode* source_code() const;

  ast::Class* FindClass(base::StringPiece name);
  ast::Node* FindMember(base::StringPiece name);
  std::string Format();
  std::string Format(base::StringPiece source_code);
  std::string GetErrors();
  std::string GetWarnings();
  bool Parse();
  void Prepare(base::StringPiece16 source_code);
  void Prepare(base::StringPiece source_code);
  std::vector<ast::Node*> QueryAstNodes(TokenType token_type);

 private:
  const std::unique_ptr<CompilationSession> session_;
  std::vector<std::unique_ptr<StringSourceCode>> source_codes_;

  DISALLOW_COPY_AND_ASSIGN(CompilerTest);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TESTING_COMPILER_TEST_H_
