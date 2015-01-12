// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/analyzer_test.h"

#include "base/macros.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/source_code_range.h"
#include "elang/compiler/token_data.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// NameResolverTest
//
class NameResolverTest : public testing::AnalyzerTest {
 protected:
  NameResolverTest();

  ast::NameReference* NewNameReference(TokenType type);
  Token* NewToken(TokenType type);

 private:
  NameResolver name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(NameResolverTest);
};

NameResolverTest::NameResolverTest() : name_resolver_(session()) {
}

ast::NameReference* NameResolverTest::NewNameReference(TokenType token_type) {
  return session()->ast_factory()->NewNameReference(NewToken(token_type));
}

Token* NameResolverTest::NewToken(TokenType token_type) {
  return session()->NewToken(SourceCodeRange(), TokenData(token_type));
}

TEST_F(NameResolverTest, SystemInt32) {
  auto const system_ast_ns = FindMember("System.Int32");
  ASSERT_TRUE(system_ast_ns) << "namespace System not found";
  auto const int32_ast_class = FindMember("System.Int32");
  ASSERT_TRUE(int32_ast_class) << "class System.Int32 isn't installed.";
  auto const int32_class = name_resolver()->Resolve(int32_ast_class);
  EXPECT_TRUE(int32_class) << "class System.Int32 isn't resolved.";
}

}  // namespace
}  // namespace compiler
}  // namespace elang
