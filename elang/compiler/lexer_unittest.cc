// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/lexer.h"

#include "base/macros.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {

namespace {
class TestLexer {
  private: CompilationSession session_;
  private: StringSourceCode source_code_;
  private: CompilationUnit compilation_unit_;
  private: Lexer lexer_;

  public: TestLexer(const base::string16& source);
  public: Token Get();
  public: Token MakeToken(TokenType type, int start, int end);

  DISALLOW_COPY_AND_ASSIGN(TestLexer);
};

TestLexer::TestLexer(const base::string16& string)
    : source_code_(L"sample", string),
      compilation_unit_(&source_code_),
      lexer_(&session_, &compilation_unit_) {
}

Token TestLexer::Get() {
  return lexer_.GetToken();
}

Token TestLexer::MakeToken(TokenType type, int start, int end) {
  return Token(SourceCodeRange(&source_code_, start, end), type);
}

}  // namespace

bool operator==(const Token& token1, const Token& token2) {
  return token1.type() == token2.type() &&
         token1.location() == token2.location();
}

void PrintTo(const Token& token, ::std::ostream* ostream) {
  static const char* kTokenTypeString[] = {
    #define V(name, string, details) string,
    TOKEN_LIST(V, V)
  };
  *ostream << "Token(" << kTokenTypeString[static_cast<int>(token.type())] <<
      " " << token.location().start().offset() <<
      " " << token.location().end().offset() << ")";
}

#define EXPECT_TOKEN(type, start, end) \
  EXPECT_EQ(lexer.MakeToken(TokenType::type, start, end), lexer.Get())

TEST(LexerTest, Basic) {
  TestLexer lexer(
    // 012345678901234
     L"class Foo<T> {\n"
   //  5678901234567890
     L"  var ch = 'c';\n"
   //  1234567890123456789
     L"  var ival = 1234;\n"
   //  012345678901 2345678 901
     L"  var str = \"string\";\n"
   //  2345678901234567890123456789012
     L"  T method(T? p) { return p; }\n"
   //  3
     L"}");
  // Lien 0
  EXPECT_TOKEN(Class, 0, 5);
  EXPECT_TOKEN(SimpleName, 6, 9);
  EXPECT_TOKEN(LeftAngleBracket, 9, 10);
  EXPECT_TOKEN(SimpleName, 10, 11);
  EXPECT_TOKEN(Gt, 11, 12);
  EXPECT_TOKEN(LeftCurryBracket, 13, 14);

  // Line 1
  EXPECT_TOKEN(Var, 17, 20);
  EXPECT_TOKEN(SimpleName, 21, 23);
  EXPECT_TOKEN(Assign, 24, 25);
  EXPECT_TOKEN(CharacterLiteral, 26, 29);
  EXPECT_TOKEN(SemiColon, 29, 30);

  // Line 2
  EXPECT_TOKEN(Var, 33, 36);
  EXPECT_TOKEN(SimpleName, 37, 41);
  EXPECT_TOKEN(Assign, 42, 43);
  EXPECT_TOKEN(Int32Literal, 44, 48);
  EXPECT_TOKEN(SemiColon, 48, 49);

  // Line 3
  EXPECT_TOKEN(Var, 52, 55);
  EXPECT_TOKEN(SimpleName, 56, 59);
  EXPECT_TOKEN(Assign, 60, 61);
  EXPECT_TOKEN(StringLiteral, 62, 70);
  EXPECT_TOKEN(SemiColon, 70, 71);

  // Line 4
  EXPECT_TOKEN(SimpleName, 74, 75);
  EXPECT_TOKEN(SimpleName, 76, 82);
  EXPECT_TOKEN(LeftParenthesis, 82, 83);
  EXPECT_TOKEN(SimpleName, 83, 84);
  EXPECT_TOKEN(OptionalType, 84, 85);
  EXPECT_TOKEN(SimpleName, 86, 87);
  EXPECT_TOKEN(RightParenthesis, 87, 88);
  EXPECT_TOKEN(LeftCurryBracket, 89, 90);
  EXPECT_TOKEN(Return, 91, 97);
  EXPECT_TOKEN(SimpleName, 98, 99);
  EXPECT_TOKEN(SemiColon, 99, 100);
  EXPECT_TOKEN(RightCurryBracket, 101, 102);

  // Line 5
  EXPECT_TOKEN(RightCurryBracket, 103, 104);

  EXPECT_TOKEN(EndOfSource, 103, 104);

  // We should get |EndOfSource| once we are at end of source.
  EXPECT_TOKEN(EndOfSource, 103, 104);
}

}  // namespace compiler
}  // namespace elang
