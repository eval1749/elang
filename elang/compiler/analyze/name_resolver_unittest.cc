// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/compiler_test.h"

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
class NameResolverTest : public testing::CompilerTest {
 protected:
  NameResolverTest();

  NameResolver* resolver() { return &name_resolver_; }

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

TEST_F(NameResolverTest, Basic) {
  auto const int_name_ref = NewNameReference(TokenType::Int);
  auto const int32_class = resolver()->FindReference(int_name_ref);
  ASSERT_TRUE(int32_class);
  EXPECT_EQ(int32_class->name()->simple_name(),
            session()->NewAtomicString(L"Int32"));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
