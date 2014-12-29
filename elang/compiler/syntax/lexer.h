// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SYNTAX_LEXER_H_
#define ELANG_COMPILER_SYNTAX_LEXER_H_

#include <memory>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/types.h"
#include "elang/compiler/character_stream.h"

namespace elang {
namespace compiler {

class CompilationSession;
class CompilationUnit;
enum class ErrorCode;
class SourceCodeRange;
class Token;
class TokenData;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// Lexer
//
class Lexer final {
 public:
  Lexer(CompilationSession* session, CompilationUnit* compilation_unit);
  ~Lexer();

  Token* GetToken();

 private:
  class CharSink;
  class InputStream;
  enum class State;

  const std::unique_ptr<CharSink> char_sink_;
  CompilationUnit* const compilation_unit_;
  std::unique_ptr<InputStream> input_stream_;
  int token_end_;
  int token_start_;
  CompilationSession* const session_;

  void Advance();
  bool AdvanceIf(base::char16 char_code);
  bool AdvanceIfEither(base::char16 char_code1, base::char16 char_code2);
  SourceCodeRange ComputeLocation();
  SourceCodeRange ComputeLocation(int length);
  Token* Error(ErrorCode error_code);
  Token* HandleAfterDecimalPoint(uint64_t int_part);
  Token* HandleAtMark();
  Token* HandleExponent(uint64_t u64, int exponent_offset);
  Token* HandleInteger(int base);
  Token* HandleIntegerOrReal(int digit);
  Token* HandleIntegerSuffix(uint64_t value);
  Token* HandleMayBeEq(TokenType with_eq, TokenType without_eq);
  Token* HandleName(base::char16 char_code);
  Token* HandleOneChar(TokenType token_type);
  Token* HandleStringLiteral(base::char16 quote_code);
  Token* HandleZero();
  bool IsAtEndOfStream();
  Token* NewFloatLiteral(TokenType token_type, uint64_t u64, int exponent);
  Token* NewIntLiteral(TokenType token_type, uint64_t u64);
  Token* NewToken(TokenType token_type);
  Token* NewToken(const TokenData& token_data);
  base::char16 PeekChar();
  void SkipLineComment();
  bool SkipBlockComment();
  base::char16 ReadChar();

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SYNTAX_LEXER_H_
