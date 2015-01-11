// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TESTING_COMPILER_TEST_H_
#define ELANG_COMPILER_TESTING_COMPILER_TEST_H_

#include <memory>
#include <string>

#include "base/strings/string_piece.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {

namespace ast {
class Class;
class NamespaceMember;
}

class CompilationSession;
class NameResolver;
class SourceCode;
class StringSourceCode;

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// CompilerTest is a simple harness for testing interactions with compiler.
//
class CompilerTest : public ::testing::Test {
 protected:
  CompilerTest();
  ~CompilerTest() override;

  NameResolver* name_resolver() const { return name_resolver_.get(); }
  CompilationSession* session() const { return session_.get(); }
  SourceCode* source_code() const;

  std::string AnalyzeNamespace();
  ast::Class* FindClass(base::StringPiece name);
  ast::NamespaceMember* FindMember(base::StringPiece name);
  std::string Format();
  std::string Format(base::StringPiece source_code);
  std::string GetBaseClasses(base::StringPiece name);
  std::string GetErrors();
  std::string GetWarnings();
  bool Parse();
  void PopulateSystemNamespace();
  void Prepare(base::StringPiece16 source_code);
  void Prepare(base::StringPiece source_code);

 private:
  const std::unique_ptr<CompilationSession> session_;

  std::unique_ptr<NameResolver> name_resolver_;
  std::unique_ptr<StringSourceCode> source_code_;

  DISALLOW_COPY_AND_ASSIGN(CompilerTest);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TESTING_COMPILER_TEST_H_
