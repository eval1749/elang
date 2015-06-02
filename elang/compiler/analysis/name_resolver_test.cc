// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/analyzer_test.h"

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/source_code_range.h"
#include "elang/compiler/string_source_code.h"
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
  NameResolverTest() = default;

  Token* NewKeyword(TokenType type);
  Token* NewName(base::StringPiece name);
  ast::Type* NewTypeReference(TokenType keyword);
  ast::Type* NewTypeReference(base::StringPiece name);

 private:
  DISALLOW_COPY_AND_ASSIGN(NameResolverTest);
};

Token* NameResolverTest::NewKeyword(TokenType type) {
  static const char* const keywords[] = {
#define V(Name, string, details) string,
      FOR_EACH_TOKEN(V, V)
#undef V
  };
  auto const name = session()->NewAtomicString(
      base::UTF8ToUTF16(keywords[static_cast<int>(type)]));
  return session()->NewToken(SourceCodeRange(), TokenData(type, name));
}

Token* NameResolverTest::NewName(base::StringPiece name) {
  return session()->NewToken(
      SourceCodeRange(), session()->NewAtomicString(base::UTF8ToUTF16(name)));
}

ast::Type* NameResolverTest::NewTypeReference(TokenType keyword) {
  return session()->ast_factory()->NewTypeNameReference(
      session()->ast_factory()->NewNameReference(NewKeyword(keyword)));
}

ast::Type* NameResolverTest::NewTypeReference(base::StringPiece reference) {
  ast::Type* last = nullptr;
  for (size_t pos = 0; pos < reference.size(); ++pos) {
    auto dot_pos = reference.find('.', pos);
    if (dot_pos == base::StringPiece::npos)
      dot_pos = reference.size();
    auto const name = NewName(reference.substr(pos, dot_pos - pos));
    pos = dot_pos;
    if (last) {
      last = session()->ast_factory()->NewTypeMemberAccess(
          session()->ast_factory()->NewMemberAccess(last, name));
      continue;
    }
    last = session()->ast_factory()->NewTypeNameReference(
        session()->ast_factory()->NewNameReference(name));
  }
  return last;
}

TEST_F(NameResolverTest, SystemInt32) {
  Prepare("");
  ASSERT_EQ("", Analyze());
  auto const ref = NewTypeReference("System.Int32");
  auto const context = compilation_units().front()->namespace_body();
  auto const node = name_resolver()->ResolveReference(ref, context);
  EXPECT_EQ("System.Int32", ToString(node));
}

}  // namespace
}  // namespace compiler
}  // namespace elang
