// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "base/logging.h"
#include "base/macros.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/syntax/lexer.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"
#include "elang/compiler/testing/compiler_test.h"

namespace elang {
namespace compiler {

namespace {

//////////////////////////////////////////////////////////////////////
//
// LexerTest
//
class LexerTest : public testing::CompilerTest {
 protected:
  LexerTest();
  ~LexerTest() override = default;

  Token* Get();
  Token* MakeToken(int start, int end, float32_t f32);
  Token* MakeToken(int start, int end, float64_t f64);
  Token* MakeToken(TokenType type,
                   int start,
                   int end,
                   base::StringPiece16 data);
  Token* MakeToken(TokenType type, int start, int end, uint64_t u64);
  Token* Peek();
  void PrepareLexer(base::StringPiece16 source_text);

 private:
  std::unique_ptr<Lexer> lexer_;
  Token* token_;

  DISALLOW_COPY_AND_ASSIGN(LexerTest);
};

LexerTest::LexerTest() : token_(nullptr) {
}

Token* LexerTest::Get() {
  return token_ = lexer_->GetToken();
}

Token* LexerTest::MakeToken(int start, int end, float32_t f32) {
  auto location = SourceCodeRange(source_code(), start, end);
  return session()->NewToken(location, TokenData(f32));
}

Token* LexerTest::MakeToken(int start, int end, float64_t f64) {
  auto location = SourceCodeRange(source_code(), start, end);
  return session()->NewToken(location, TokenData(f64));
}

Token* LexerTest::MakeToken(TokenType type,
                            int start,
                            int end,
                            base::StringPiece16 data) {
  auto location = SourceCodeRange(source_code(), start, end);
  if (type == TokenType::StringLiteral)
    return session()->NewToken(location, TokenData(session()->NewString(data)));
  auto const simple_name = session()->NewAtomicString(data);
  return session()->NewToken(location, TokenData(type, simple_name));
}

Token* LexerTest::MakeToken(TokenType type, int start, int end, uint64_t u64) {
  DCHECK_NE(type, TokenType::SimpleName);
  auto location = SourceCodeRange(source_code(), start, end);
  switch (type) {
    case TokenType::CharacterLiteral:
    case TokenType::Int32Literal:
    case TokenType::Int64Literal:
    case TokenType::UInt32Literal:
    case TokenType::UInt64Literal:
      return session()->NewToken(location, TokenData(type, u64));
  }
  return session()->NewToken(location, TokenData(type));
}

Token* LexerTest::Peek() {
  DCHECK(token_);
  return token_;
}

void LexerTest::PrepareLexer(base::StringPiece16 source_text) {
  Prepare(source_text);
  auto const compilation_unit = session()->NewCompilationUnit(source_code());
  lexer_.reset(new Lexer(session(), compilation_unit));
}

std::string ToString(Token* token) {
  std::stringstream ostream;
  ostream << *token;
  return ostream.str();
}

}  // namespace

char ToHexDigit(int n) {
  return "0123456789ABCDEF"[n & 15];
}

void PrintTo(const Token& token, ::std::ostream* ostream) {
  *ostream << token.data() << "(" << token.location().start().offset() << " "
           << token.location().end().offset() << ")";
}

#define EXPECT_FLOAT_TOKEN(start, end, data) \
  EXPECT_EQ(*MakeToken(start, end, data), *Get())

#define EXPECT_OPERATOR_TOKEN(type, start, end) \
  EXPECT_EQ(*MakeToken(TokenType::type, start, end, 1), *Get())

#define EXPECT_TOKEN(type, start, end, data) \
  EXPECT_EQ(*MakeToken(TokenType::type, start, end, data), *Get())

TEST_F(LexerTest, Basic) {
  PrepareLexer(
      L"class Foo<T> {\n"
      L"  var ch = 'c';\n"
      L"  var ival = 1234;\n"
      L"  var str = \"string\";\n"
      L"  T method(T? p) { return p; }\n"
      //   3
      L"}");
  // Lien 0
  EXPECT_TOKEN(Class, 0, 5, L"class");
  EXPECT_TOKEN(SimpleName, 6, 9, L"Foo");
  EXPECT_TOKEN(LeftAngleBracket, 9, 10, '<');
  EXPECT_TOKEN(SimpleName, 10, 11, L"T");
  EXPECT_TOKEN(RightAngleBracket, 11, 12, '>');
  EXPECT_TOKEN(LeftCurryBracket, 13, 14, '{');

  // Line 1
  EXPECT_TOKEN(Var, 17, 20, L"var");
  EXPECT_TOKEN(SimpleName, 21, 23, L"ch");
  EXPECT_TOKEN(Assign, 24, 25, '=');
  EXPECT_TOKEN(CharacterLiteral, 26, 29, 'c');
  EXPECT_TOKEN(SemiColon, 29, 30, ';');

  // Line 2
  EXPECT_TOKEN(Var, 33, 36, L"var");
  EXPECT_TOKEN(SimpleName, 37, 41, L"ival");
  EXPECT_TOKEN(Assign, 42, 43, '=');
  EXPECT_TOKEN(Int32Literal, 44, 48, 1234);
  EXPECT_TOKEN(SemiColon, 48, 49, ';');

  // Line 3
  EXPECT_TOKEN(Var, 52, 55, L"var");
  EXPECT_TOKEN(SimpleName, 56, 59, L"str");
  EXPECT_TOKEN(Assign, 60, 61, '=');
  EXPECT_TOKEN(StringLiteral, 62, 70, L"string");
  EXPECT_TOKEN(SemiColon, 70, 71, ';');

  // Line 4
  EXPECT_TOKEN(SimpleName, 74, 75, L"T");
  EXPECT_TOKEN(SimpleName, 76, 82, L"method");
  EXPECT_TOKEN(LeftParenthesis, 82, 83, '(');
  EXPECT_TOKEN(SimpleName, 83, 84, L"T");
  EXPECT_TOKEN(OptionalType, 84, 85, '?');
  EXPECT_TOKEN(SimpleName, 86, 87, L"p");
  EXPECT_TOKEN(RightParenthesis, 87, 88, ')');
  EXPECT_TOKEN(LeftCurryBracket, 89, 90, '{');
  EXPECT_TOKEN(Return, 91, 97, L"return");
  EXPECT_TOKEN(SimpleName, 98, 99, L"p");
  EXPECT_TOKEN(SemiColon, 99, 100, 'l');
  EXPECT_TOKEN(RightCurryBracket, 101, 102, '}');

  // Line 5
  EXPECT_TOKEN(RightCurryBracket, 103, 104, '}');

  EXPECT_TOKEN(EndOfSource, 103, 104, 0);

  // We should get |EndOfSource| once we are at end of source.
  EXPECT_TOKEN(EndOfSource, 103, 104, 0);
}

TEST_F(LexerTest, BlockComment) {
  PrepareLexer(L"/* foo */|");
  EXPECT_OPERATOR_TOKEN(BitOr, 9, 10);
}

TEST_F(LexerTest, Bracket) {
  PrepareLexer(L"()<>[]{}");
  EXPECT_TRUE(Get()->is_left_bracket()) << "(";
  EXPECT_TRUE(Peek()->right_bracket() == TokenType::RightParenthesis);
  EXPECT_TRUE(Get()->is_right_bracket()) << ")";
  EXPECT_TRUE(Get()->is_left_bracket()) << "<";
  EXPECT_TRUE(Peek()->right_bracket() == TokenType::RightAngleBracket);
  EXPECT_TRUE(Get()->is_right_bracket()) << ">";
  EXPECT_TRUE(Get()->is_left_bracket()) << "[";
  EXPECT_TRUE(Peek()->right_bracket() == TokenType::RightSquareBracket);
  EXPECT_TRUE(Get()->is_right_bracket()) << "]";
  EXPECT_TRUE(Get()->is_left_bracket()) << "{";
  EXPECT_TRUE(Peek()->right_bracket() == TokenType::RightCurryBracket);
  EXPECT_TRUE(Get()->is_right_bracket()) << "}";
}

TEST_F(LexerTest, Integers) {
  PrepareLexer(
      L" 1234   0b0101   0o177   0x7FE5         "  // 0
      L" 1234l  0b0101l  0o177l  0x7FE5l        "  // 40
      L" 1234u  0b0101u  0o177u  0x7FE5u        "  // 80
      L" 1234lu 0b0101Lu 0o177lU 0x7FE5Lu       "  // 120
      L" 1234ul 0b0101UL 0o177uL 0x7FE5Ul       "  // 160
      L"");
  EXPECT_TOKEN(Int32Literal, 1, 5, 1234);
  EXPECT_TOKEN(Int32Literal, 8, 14, 5);
  EXPECT_TOKEN(Int32Literal, 17, 22, 127);
  EXPECT_TOKEN(Int32Literal, 25, 31, 0x7FE5);

  EXPECT_TOKEN(Int64Literal, 41, 46, 1234);
  EXPECT_TOKEN(Int64Literal, 48, 55, 5);
  EXPECT_TOKEN(Int64Literal, 57, 63, 127);
  EXPECT_TOKEN(Int64Literal, 65, 72, 0x7FE5);

  EXPECT_TOKEN(UInt32Literal, 81, 86, 1234);
  EXPECT_TOKEN(UInt32Literal, 88, 95, 5);
  EXPECT_TOKEN(UInt32Literal, 97, 103, 127);
  EXPECT_TOKEN(UInt32Literal, 105, 112, 0x7FE5);

  EXPECT_TOKEN(UInt64Literal, 121, 127, 1234);
  EXPECT_TOKEN(UInt64Literal, 128, 136, 5);
  EXPECT_TOKEN(UInt64Literal, 137, 144, 127);
  EXPECT_TOKEN(UInt64Literal, 145, 153, 0x7FE5);

  EXPECT_TOKEN(UInt64Literal, 161, 167, 1234);
  EXPECT_TOKEN(UInt64Literal, 168, 176, 5);
  EXPECT_TOKEN(UInt64Literal, 177, 184, 127);
  EXPECT_TOKEN(UInt64Literal, 185, 193, 0x7FE5);
}

TEST_F(LexerTest, LineComment) {
  PrepareLexer(L"// foo\n|");
  EXPECT_OPERATOR_TOKEN(BitOr, 7, 8);
}

TEST_F(LexerTest, Operators) {
  PrepareLexer(
      L" ~ . ,    "
      L" * *= / /="
      L" % %= ^ ^="
      L" ? ?? ! !="
      L" + += ++  "
      L" - -= --  "
      L" & &= &&  "
      L" | |= ||  "
      L" = == =>  "
      L" < <= <<  "
      L" <<=      "
      L" > >= >>  "
      L" >>=");
  EXPECT_OPERATOR_TOKEN(BitNot, 1, 2);
  EXPECT_OPERATOR_TOKEN(Dot, 3, 4);
  EXPECT_OPERATOR_TOKEN(Comma, 5, 6);

  EXPECT_OPERATOR_TOKEN(Mul, 11, 12);
  EXPECT_OPERATOR_TOKEN(MulAssign, 13, 15);
  EXPECT_OPERATOR_TOKEN(Div, 16, 17);
  EXPECT_OPERATOR_TOKEN(DivAssign, 18, 20);

  EXPECT_OPERATOR_TOKEN(Mod, 21, 22);
  EXPECT_OPERATOR_TOKEN(ModAssign, 23, 25);
  EXPECT_OPERATOR_TOKEN(BitXor, 26, 27);
  EXPECT_OPERATOR_TOKEN(BitXorAssign, 28, 30);

  EXPECT_OPERATOR_TOKEN(QuestionMark, 31, 32);
  EXPECT_OPERATOR_TOKEN(NullOr, 33, 35);
  EXPECT_OPERATOR_TOKEN(Not, 36, 37);
  EXPECT_OPERATOR_TOKEN(Ne, 38, 40);

  EXPECT_OPERATOR_TOKEN(Add, 41, 42);
  EXPECT_OPERATOR_TOKEN(AddAssign, 43, 45);
  EXPECT_OPERATOR_TOKEN(Increment, 46, 48);

  EXPECT_OPERATOR_TOKEN(Sub, 51, 52);
  EXPECT_OPERATOR_TOKEN(SubAssign, 53, 55);
  EXPECT_OPERATOR_TOKEN(Decrement, 56, 58);

  EXPECT_OPERATOR_TOKEN(BitAnd, 61, 62);
  EXPECT_OPERATOR_TOKEN(BitAndAssign, 63, 65);
  EXPECT_OPERATOR_TOKEN(And, 66, 68);

  EXPECT_OPERATOR_TOKEN(BitOr, 71, 72);
  EXPECT_OPERATOR_TOKEN(BitOrAssign, 73, 75);
  EXPECT_OPERATOR_TOKEN(Or, 76, 78);

  EXPECT_OPERATOR_TOKEN(Assign, 81, 82);
  EXPECT_OPERATOR_TOKEN(Eq, 83, 85);
  EXPECT_OPERATOR_TOKEN(Arrow, 86, 88);

  EXPECT_OPERATOR_TOKEN(Lt, 91, 92);
  EXPECT_OPERATOR_TOKEN(Le, 93, 95);
  EXPECT_OPERATOR_TOKEN(Shl, 96, 98);
  EXPECT_OPERATOR_TOKEN(ShlAssign, 101, 104);

  EXPECT_OPERATOR_TOKEN(Gt, 111, 112);
  EXPECT_OPERATOR_TOKEN(Ge, 113, 115);
  EXPECT_OPERATOR_TOKEN(Shr, 116, 118);
  EXPECT_OPERATOR_TOKEN(ShrAssign, 121, 124);
}

TEST_F(LexerTest, Reals) {
  PrepareLexer(
      L"    0.0  1.34  2.5E10  3.5e+10  46E-11  "
      L"    0.0f 1.34f 2.5E10F 3.5e+10F 46E-11f ");
  EXPECT_FLOAT_TOKEN(4, 7, 0.0);
  EXPECT_FLOAT_TOKEN(9, 13, 1.34);
  EXPECT_FLOAT_TOKEN(15, 21, 2.5E10);
  EXPECT_FLOAT_TOKEN(23, 30, 3.5E10);
  EXPECT_FLOAT_TOKEN(32, 38, 46E-11);

  EXPECT_FLOAT_TOKEN(44, 48, 0.0f);
  EXPECT_FLOAT_TOKEN(49, 54, 1.34f);
  EXPECT_FLOAT_TOKEN(55, 62, 2.5E10f);
  EXPECT_FLOAT_TOKEN(63, 71, 3.5E10f);
  EXPECT_FLOAT_TOKEN(72, 79, 46E-11f);
}

TEST_F(LexerTest, CharsBasic) {
  PrepareLexer(L"'c'");
  EXPECT_TOKEN(CharacterLiteral, 0, 3, 'c');
}

TEST_F(LexerTest, CharsBackslash) {
  PrepareLexer(L"'\\n'");
  EXPECT_TOKEN(CharacterLiteral, 0, 4, '\n');
}

TEST_F(LexerTest, CharsUnicode) {
  PrepareLexer(L"'\u1234'");
  EXPECT_TOKEN(CharacterLiteral, 0, 3, 0x1234);
}

TEST_F(LexerTest, StringsBackslash) {
  PrepareLexer(L"\"\\a\\b\\t\\n\\v\\f\\r\\u1234x\uABCD\"");
  EXPECT_TOKEN(StringLiteral, 0, 24, L"\a\b\t\n\v\f\r\u1234x\uABCD");
}

TEST_F(LexerTest, VerbatimName) {
  PrepareLexer(L"@x @abc @class");

  auto const name_x = Get();
  ASSERT_EQ(TokenType::VerbatimName, name_x->type());
  EXPECT_TRUE(name_x->is_name());
  EXPECT_EQ(L"x", name_x->atomic_string()->string());
  EXPECT_EQ("@x", ToString(name_x));

  auto const name_abc = Get();
  ASSERT_EQ(TokenType::VerbatimName, name_abc->type());
  EXPECT_TRUE(name_abc->is_name());
  EXPECT_EQ(L"abc", name_abc->atomic_string()->string());
  EXPECT_EQ("@abc", ToString(name_abc));

  auto const name_class = Get();
  ASSERT_EQ(TokenType::VerbatimName, name_class->type());
  EXPECT_TRUE(name_class->is_name());
  EXPECT_EQ(L"class", name_class->atomic_string()->string());
  EXPECT_EQ("@class", ToString(name_class));
}

TEST_F(LexerTest, VerbatimString) {
  PrepareLexer(L"@\"ab\"\"cd\"");
  EXPECT_TOKEN(StringLiteral, 0, 9, L"ab\"cd");
}

}  // namespace compiler
}  // namespace elang
