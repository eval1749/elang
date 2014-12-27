// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_lexer_h)
#define INCLUDE_elang_compiler_lexer_h

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
  private: class CharSink;
  private: class InputStream;
  private: enum class State;

  private: const std::unique_ptr<CharSink> char_sink_;
  private: CompilationUnit* const compilation_unit_;
  private: std::unique_ptr<InputStream> input_stream_;
  private: int token_end_;
  private: int token_start_;
  private: CompilationSession* const session_;

  public: Lexer(CompilationSession* session, CompilationUnit* compilation_unit);
  public: ~Lexer();

  private: void Advance();
  private: bool AdvanceIf(base::char16 char_code);
  private: bool AdvanceIfEither(base::char16 char_code1, base::char16 char_code2);
  private: SourceCodeRange ComputeLocation();
  private: SourceCodeRange ComputeLocation(int length);
  private: Token* Error(ErrorCode error_code);
  public: Token* GetToken();
  private: Token* HandleAfterDecimalPoint(uint64_t int_part);
  private: Token* HandleAtMark();
  private: Token* HandleExponent(uint64_t u64, int exponent_offset);
  private: Token* HandleInteger(int base);
  private: Token* HandleIntegerOrReal(int digit);
  private: Token* HandleIntegerSuffix(uint64_t value);
  private: Token* HandleMayBeEq(TokenType with_eq, TokenType without_eq);
  private: Token* HandleName(base::char16 char_code);
  private: Token* HandleOneChar(TokenType token_type);
  private: Token* HandleStringLiteral(base::char16 quote_code);
  private: Token* HandleZero();
  private: bool IsAtEndOfStream();
  private: Token* NewFloatLiteral(TokenType token_type, uint64_t u64,
                                 int exponent);
  private: Token* NewIntLiteral(TokenType token_type, uint64_t u64);
  private: Token* NewToken(TokenType token_type);
  private: Token* NewToken(const TokenData& token_data);
  private: base::char16 PeekChar();
  private: void SkipLineComment();
  private: bool SkipBlockComment();
  private: base::char16 ReadChar();

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_lexer_h)
