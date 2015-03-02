// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/syntax/lexer.h"

#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {

TokenType ComputeToken(AtomicString* name) {
  typedef std::unordered_map<base::StringPiece16, TokenType> KeywordMap;
  CR_DEFINE_STATIC_LOCAL(KeywordMap*, keyword_map, ());
  if (!keyword_map) {
    keyword_map = new KeywordMap();
#define K(name, string, details) (*keyword_map)[L##string] = TokenType::name;
    FOR_EACH_TOKEN(IGNORE_TOKEN, K)
#undef K
  }
  auto it = keyword_map->find(name->string());
  return it == keyword_map->end() ? TokenType::SimpleName : it->second;
}

int DigitToInt(base::char16 char_code, int base) {
  auto const value = char_code - '0';
  if (value >= 0 && value < base)
    return value;
  if (base <= 10)
    return -1;
  auto const alpha = char_code - 'A' + 10;
  if (alpha >= 10 && alpha < base)
    return alpha;
  auto const alpha2 = char_code - 'a' + 10;
  if (alpha2 >= 10 && alpha2 < base)
    return alpha2;
  return -1;
}

bool IsNameStartChar(base::char16 char_code) {
  if (char_code >= 'A' && char_code <= 'Z')
    return true;
  if (char_code >= 'a' && char_code <= 'z')
    return true;
  return char_code == '_';
}

bool IsNameChar(base::char16 char_code) {
  if (IsNameStartChar(char_code))
    return true;
  return char_code >= '0' && char_code <= '9';
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Lexer::CharSink
//
class Lexer::CharSink {
 public:
  CharSink();
  ~CharSink() = default;

  base::StringPiece16 End();
  void AddChar(base::char16 char_code);
  void Start();

 private:
  std::vector<base::char16> buffer_;

  DISALLOW_COPY_AND_ASSIGN(CharSink);
};

Lexer::CharSink::CharSink() : buffer_(200) {
}

void Lexer::CharSink::AddChar(base::char16 char_code) {
  buffer_.push_back(char_code);
}

base::StringPiece16 Lexer::CharSink::End() {
  return base::StringPiece16(buffer_.data(), buffer_.size());
}

void Lexer::CharSink::Start() {
  buffer_.resize(0);
}

//////////////////////////////////////////////////////////////////////
//
// Lexer::InputStream
//
class Lexer::InputStream {
 public:
  explicit InputStream(SourceCode* source_code);
  ~InputStream();

  void Advance();
  bool IsAtEndOfStream();
  base::char16 PeekChar();
  base::char16 ReadChar();

 private:
  bool has_char_;
  base::char16 last_char_;
  int offset_;
  SourceCode* source_code_;
  CharacterStream* const stream_;

  DISALLOW_COPY_AND_ASSIGN(InputStream);
};

Lexer::InputStream::InputStream(SourceCode* source_code)
    : has_char_(false),
      last_char_(0),
      offset_(0),
      source_code_(source_code),
      stream_(source_code->GetStream()) {
}

Lexer::InputStream::~InputStream() {
}

void Lexer::InputStream::Advance() {
  has_char_ = false;
  if (IsAtEndOfStream())
    return;
  ReadChar();
}

bool Lexer::InputStream::IsAtEndOfStream() {
  return !has_char_ && stream_->IsAtEndOfStream();
}

base::char16 Lexer::InputStream::PeekChar() {
  if (has_char_)
    return last_char_;
  return ReadChar();
}

base::char16 Lexer::InputStream::ReadChar() {
  DCHECK(!has_char_);
  DCHECK(!stream_->IsAtEndOfStream());
  ++offset_;
  has_char_ = true;
  last_char_ = stream_->ReadChar();
  if (last_char_ == '\n')
    source_code_->RememberStartOfLine(offset_ + 1);
  return last_char_;
}

//////////////////////////////////////////////////////////////////////
//
// Lexer
//
Lexer::Lexer(CompilationSession* session, CompilationUnit* compilation_unit)
    : char_sink_(new CharSink()),
      compilation_unit_(compilation_unit),
      input_stream_(new InputStream(compilation_unit->source_code())),
      session_(session),
      token_end_(0),
      token_start_(0) {
}

Lexer::~Lexer() {
}

void Lexer::Advance() {
  ++token_end_;
  input_stream_->Advance();
}

bool Lexer::AdvanceIf(base::char16 char_code) {
  if (IsAtEndOfStream())
    return false;
  if (PeekChar() != char_code)
    return false;
  Advance();
  return true;
}

bool Lexer::AdvanceIfEither(base::char16 char_code1, base::char16 char_code2) {
  if (IsAtEndOfStream())
    return false;
  if (PeekChar() != char_code1 && PeekChar() != char_code2)
    return false;
  Advance();
  return true;
}

SourceCodeRange Lexer::ComputeLocation() {
  return ComputeLocation(token_end_ - token_start_);
}

SourceCodeRange Lexer::ComputeLocation(int length) {
  return SourceCodeRange(compilation_unit_->source_code(), token_start_,
                         token_start_ + length);
}

Token* Lexer::Error(ErrorCode error_code) {
  session_->AddError(ComputeLocation(), error_code);
  return HandleOneChar(TokenType::Illegal);
}

Token* Lexer::GetToken() {
  auto just_after_whitespace = false;
  for (;;) {
    if (IsAtEndOfStream())
      return HandleOneChar(TokenType::EndOfSource);
    auto const char_code = PeekChar();
    Advance();
    if (char_code == ' ' || char_code == 0x0D || char_code == 0x0A) {
      just_after_whitespace = true;
      continue;
    }
    token_start_ = token_end_ - 1;
    if (char_code < ' ')
      return HandleOneChar(TokenType::Illegal);
    switch (char_code) {
      case '!':
        return HandleMayBeEq(TokenType::Ne, TokenType::Not);
      case '"':
      case '\'':
        return HandleStringLiteral(char_code);
      case '%':
        return HandleMayBeEq(TokenType::ModAssign, TokenType::Mod);
      case '&':
        if (AdvanceIf('&'))
          return NewToken(TokenType::And);
        return HandleMayBeEq(TokenType::BitAndAssign, TokenType::BitAnd);
      case '(':
        return HandleOneChar(TokenType::LeftParenthesis);
      case ')':
        return HandleOneChar(TokenType::RightParenthesis);
      case '*':
        return HandleMayBeEq(TokenType::MulAssign, TokenType::Mul);
      case '+':
        if (AdvanceIf('+'))
          return NewToken(TokenType::Increment);
        return HandleMayBeEq(TokenType::AddAssign, TokenType::Add);
      case ',':
        return HandleOneChar(TokenType::Comma);
      case '-':
        if (AdvanceIf('-'))
          return NewToken(TokenType::Decrement);
        return HandleMayBeEq(TokenType::SubAssign, TokenType::Sub);
      case '.':
        return HandleOneChar(TokenType::Dot);
      case '/':
        if (AdvanceIf('*')) {
          if (!SkipBlockComment())
            return Error(ErrorCode::TokenBlockCommentUnclosed);
          just_after_whitespace = true;
          continue;
        }
        if (AdvanceIf('/')) {
          SkipLineComment();
          just_after_whitespace = true;
          continue;
        }
        return HandleMayBeEq(TokenType::DivAssign, TokenType::Div);
      case '0':
        return HandleZero();
      case ':':
        return HandleOneChar(TokenType::Colon);
      case ';':
        return HandleOneChar(TokenType::SemiColon);
      case '<':
        if (!just_after_whitespace)
          return HandleOneChar(TokenType::LeftAngleBracket);
        if (AdvanceIf('<'))
          return HandleMayBeEq(TokenType::ShlAssign, TokenType::Shl);
        return HandleMayBeEq(TokenType::Le, TokenType::Lt);
      case '=':
        if (AdvanceIf('>'))
          return NewToken(TokenType::Arrow);
        return HandleMayBeEq(TokenType::Eq, TokenType::Assign);
      case '>':
        if (!just_after_whitespace)
          return HandleOneChar(TokenType::RightAngleBracket);
        if (AdvanceIf('>'))
          return HandleMayBeEq(TokenType::ShrAssign, TokenType::Shr);
        return HandleMayBeEq(TokenType::Ge, TokenType::Gt);
      case '?':
        if (just_after_whitespace) {
          if (AdvanceIf('?'))
            return NewToken(TokenType::NullOr);
          return HandleOneChar(TokenType::QuestionMark);
        }
        if (AdvanceIf('.'))
          return NewToken(TokenType::OptionalDot);
        return HandleOneChar(TokenType::OptionalType);
      case '@':
        return HandleAtMark();
      case '[':
        return HandleOneChar(TokenType::LeftSquareBracket);
      case ']':
        return HandleOneChar(TokenType::RightSquareBracket);
      case '^':
        return HandleMayBeEq(TokenType::BitXorAssign, TokenType::BitXor);
      case '{':
        return HandleOneChar(TokenType::LeftCurryBracket);
      case '|':
        if (AdvanceIf('|'))
          return NewToken(TokenType::Or);
        return HandleMayBeEq(TokenType::BitOrAssign, TokenType::BitOr);
      case '}':
        return HandleOneChar(TokenType::RightCurryBracket);
      case '~':
        return HandleOneChar(TokenType::BitNot);
      default:
        if (IsNameStartChar(char_code))
          return HandleName(char_code);
        if (char_code >= '1' && char_code <= '9')
          return HandleIntegerOrReal(char_code - '0');
        return HandleOneChar(TokenType::Illegal);
    }
  }
}

Token* Lexer::HandleAfterDecimalPoint(uint64_t u64) {
  auto exponent = 0;
  while (!IsAtEndOfStream()) {
    auto const char_code = PeekChar();
    if (char_code >= '0' && char_code <= '9') {
      Advance();
      if (u64 >= std::numeric_limits<uint64_t>::max() / 10)
        return Error(ErrorCode::TokenRealTooManyDigits);
      u64 *= 10;
      u64 += char_code - '0';
      --exponent;
      continue;
    }
    if (AdvanceIfEither('e', 'E'))
      return HandleExponent(u64, exponent);
    if (AdvanceIfEither('f', 'F'))
      return NewFloatLiteral(TokenType::Float32Literal, u64, exponent);
    break;
  }
  return NewFloatLiteral(TokenType::Float64Literal, u64, exponent);
}

// Handle verbatim string or raw name
//   - raw string: '@' '"' (CharNotQuote | '""')* '"'
//   - raw name: '@' NameStartChar NameChar*
//
Token* Lexer::HandleAtMark() {
  if (IsAtEndOfStream())
    return Error(ErrorCode::TokenAtMarkInvalid);

  if (AdvanceIf('"')) {
    enum class State { Normal, Quote, } state = State::Normal;
    char_sink_->Start();
    while (!IsAtEndOfStream()) {
      auto const char_code = PeekChar();
      switch (state) {
        case State::Quote:
          if (char_code == '"') {
            Advance();
            char_sink_->AddChar('"');
            state = State::Normal;
            break;
          }
          return NewToken(TokenData(session_->NewString(char_sink_->End())));
        case State::Normal:
          Advance();
          if (char_code == '"') {
            state = State::Quote;
            break;
          }
          char_sink_->AddChar(char_code);
          break;
      }
    }
    if (state == State::Quote)
      return NewToken(TokenData(session_->NewString(char_sink_->End())));
    return Error(ErrorCode::TokenAtMarkStringUnclosed);
  }

  if (IsNameStartChar(PeekChar())) {
    char_sink_->Start();
    while (!IsAtEndOfStream()) {
      auto const char_code = PeekChar();
      if (!IsNameChar(char_code))
        break;
      Advance();
      char_sink_->AddChar(char_code);
    }
    auto const name = session_->NewAtomicString(char_sink_->End());
    DCHECK_GE(name->string().size(), 1u);
    return NewToken(TokenData(TokenType::VerbatimName, name));
  }

  return Error(ErrorCode::TokenAtMarkInvalid);
}

Token* Lexer::HandleExponent(uint64_t u64, int exponent_offset) {
  auto is_minus = false;
  if (AdvanceIf('-')) {
    is_minus = true;
  } else if (AdvanceIf('+')) {
    is_minus = false;
  }
  auto token_type = TokenType::Float64Literal;
  auto exponent = 0;
  while (!IsAtEndOfStream()) {
    auto const char_code = PeekChar();
    if (AdvanceIfEither('f', 'F')) {
      token_type = TokenType::Float32Literal;
      break;
    }
    if (char_code < '0' || char_code > '9')
      break;
    Advance();
    if (exponent > std::numeric_limits<uint64_t>::max() / 10) {
      return Error(ErrorCode::TokenFloatExponentOverflow);
    }
    exponent *= 10;
    exponent += char_code - '0';
  }
  if (is_minus)
    exponent = -exponent;
  exponent += exponent_offset;
  return NewFloatLiteral(token_type, u64, exponent);
}

Token* Lexer::HandleInteger(int base) {
  uint64_t u64 = 0;
  auto digit_count = 0;
  while (!IsAtEndOfStream()) {
    auto const digit = DigitToInt(PeekChar(), base);
    if (digit < 0) {
      if (!digit_count) {
        Advance();
        break;
      }
      return HandleIntegerSuffix(u64);
    }
    Advance();
    if (u64 >= std::numeric_limits<uint64_t>::max() / 10)
      return Error(ErrorCode::TokenIntegerOverflow);
    u64 *= base;
    u64 += digit;
    ++digit_count;
  }
  return Error(ErrorCode::TokenIntegerInvalid);
}

Token* Lexer::HandleIntegerOrReal(int digit) {
  uint64_t u64 = digit;
  while (!IsAtEndOfStream()) {
    auto const char_code = PeekChar();
    if (char_code >= '0' && char_code <= '9') {
      Advance();
      if (u64 >= std::numeric_limits<uint64_t>::max() / 10)
        return Error(ErrorCode::TokenIntegerOverflow);
      u64 *= 10;
      u64 += char_code - '0';
      continue;
    }
    if (char_code == '.') {
      Advance();
      return HandleAfterDecimalPoint(u64);
    }
    if (AdvanceIfEither('e', 'E'))
      return HandleExponent(u64, 0);
    if (char_code == 'l' || char_code == 'L' || char_code == 'u' ||
        char_code == 'U') {
      return HandleIntegerSuffix(u64);
    }
    break;
  }
  if (u64 >= std::numeric_limits<int32_t>::max())
    return Error(ErrorCode::TokenIntegerOverflow);
  return NewIntLiteral(TokenType::Int32Literal, u64);
}

// Handle Integer Suffixes
//  \d+ [Ll]? [Uu]?
//  \d+ [Uu]? [Ll]?
Token* Lexer::HandleIntegerSuffix(uint64_t u64) {
  if (AdvanceIfEither('l', 'L')) {
    if (AdvanceIfEither('u', 'U'))
      return NewToken(TokenData(TokenType::UInt64Literal, u64));
    return NewIntLiteral(TokenType::Int64Literal, u64);
  }
  if (AdvanceIfEither('u', 'U')) {
    if (AdvanceIfEither('l', 'L'))
      return NewToken(TokenData(TokenType::UInt64Literal, u64));
    if (u64 > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()))
      return Error(ErrorCode::TokenIntegerOverflow);
    return NewIntLiteral(TokenType::UInt32Literal, u64);
  }
  if (u64 > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()))
    return Error(ErrorCode::TokenIntegerOverflow);
  return NewIntLiteral(TokenType::Int32Literal, u64);
}

Token* Lexer::HandleMayBeEq(TokenType with_eq, TokenType without_eq) {
  if (AdvanceIf('='))
    return NewToken(with_eq);
  return NewToken(without_eq);
}

Token* Lexer::HandleName(base::char16 first_char_code) {
  char_sink_->Start();
  char_sink_->AddChar(first_char_code);
  while (!IsAtEndOfStream()) {
    auto const char_code = PeekChar();
    if (!IsNameChar(char_code))
      break;
    Advance();
    char_sink_->AddChar(char_code);
  }
  auto const name = session_->NewAtomicString(char_sink_->End());
  return NewToken(TokenData(ComputeToken(name), name));
}

Token* Lexer::HandleOneChar(TokenType token_type) {
  return session_->NewToken(ComputeLocation(1), TokenData(token_type));
}

// E supports following backslash sequence:
//   \' \" \\ \0 \a \b \f \n \r \t \uUUUU
Token* Lexer::HandleStringLiteral(base::char16 delimiter) {
  char_sink_->Start();
  enum class State { Backslash, BackslashU, Normal, } state = State::Normal;
  auto accumulator = 0;
  auto digit_count = 0;
  while (!IsAtEndOfStream()) {
    auto char_code = PeekChar();
    Advance();
    switch (state) {
      case State::Backslash:
        switch (char_code) {
          case '"':
          case '\'':
          case '\\':
            break;
          case '0':
            char_code = static_cast<base::char16>(0x0000);
            break;
          case 'a':
            char_code = static_cast<base::char16>(0x0007);
            break;
          case 'b':
            char_code = static_cast<base::char16>(0x0008);
            break;
          case 'f':
            char_code = static_cast<base::char16>(0x000C);
            break;
          case 'n':
            char_code = static_cast<base::char16>(0x000A);
            break;
          case 'r':
            char_code = static_cast<base::char16>(0x000D);
            break;
          case 't':
            char_code = static_cast<base::char16>(0x0009);
            break;
          case 'u':
            accumulator = 0;
            digit_count = 0;
            state = State::BackslashU;
            continue;
          case 'v':
            char_code = static_cast<base::char16>(0x000B);
            break;
          default:
            return Error(ErrorCode::TokenBackslashInvalid);
        }
        char_sink_->AddChar(char_code);
        state = State::Normal;
        break;
      case State::BackslashU: {
        auto const digit = DigitToInt(char_code, 16);
        if (digit < 0)
          return Error(ErrorCode::TokenBackslashUInvalid);
        accumulator <<= 4;
        accumulator += digit;
        ++digit_count;
        if (digit_count == 4) {
          char_sink_->AddChar(static_cast<base::char16>(accumulator));
          state = State::Normal;
        }
        break;
      }
      case State::Normal:
        if (char_code == '\n')
          return Error(ErrorCode::TokenStringHasNewline);
        if (char_code == '\\') {
          state = State::Backslash;
          break;
        }
        if (char_code == delimiter) {
          auto string = session_->NewString(char_sink_->End());
          auto const token = NewToken(TokenData(string));
          if (delimiter == '"')
            return token;
          if (string->size() != 1) {
            session_->AddError(ErrorCode::TokenCharacterInvalid, token);
            return NewToken(TokenType::Illegal);
          }
          return NewToken(TokenData(TokenType::CharacterLiteral, (*string)[0]));
        }
        char_sink_->AddChar(char_code);
        break;
    }
  }
  return Error(ErrorCode::TokenStringUnclosed);
}

// Handles following numeric literals:
//   '0' '.' real
//   '0' [Bb] binary
//   '0' [Ee] real
//   '0' [Ll][Uu]? int64/uint64
//   '0' [Oo] octal
//   '0' [Uu][Ll]? uint64
//   '0' [Xx] hexadecimal
Token* Lexer::HandleZero() {
  const uint64_t zero = 0u;
  if (IsAtEndOfStream())
    return NewIntLiteral(TokenType::Int32Literal, zero);
  if (AdvanceIf('.'))
    return HandleAfterDecimalPoint(0u);
  if (AdvanceIfEither('b', 'B'))
    return HandleInteger(2);
  if (AdvanceIfEither('e', 'E'))
    return HandleExponent(0u, 0);
  if (PeekChar() == 'l' || PeekChar() == 'L')
    return HandleIntegerSuffix(zero);
  if (AdvanceIfEither('o', 'O'))
    return HandleInteger(8);
  if (PeekChar() == 'u' || PeekChar() == 'U')
    return HandleIntegerSuffix(zero);
  if (AdvanceIfEither('x', 'X'))
    return HandleInteger(16);
  if (PeekChar() >= '0' && PeekChar() <= '9') {
    Advance();
    return HandleIntegerOrReal(PeekChar() - '0');
  }
  return NewIntLiteral(TokenType::Int32Literal, zero);
}

bool Lexer::IsAtEndOfStream() {
  return input_stream_->IsAtEndOfStream();
}

Token* Lexer::NewFloatLiteral(TokenType token_type,
                              uint64_t u64,
                              int exponent) {
  if (token_type == TokenType::Float32Literal) {
    auto const int_part = static_cast<float32_t>(u64);
    if (exponent >= 0) {
      auto const f32 = int_part * std::pow(10.0f, exponent);
      return NewToken(TokenData(f32));
    }
    auto const f32 = int_part / std::pow(10.0f, -exponent);
    return NewToken(TokenData(f32));
  }

  DCHECK_EQ(token_type, TokenType::Float64Literal);
  auto const int_part = static_cast<float64_t>(u64);
  if (exponent >= 0) {
    auto const f64 = int_part * std::pow(10.0, exponent);
    return NewToken(TokenData(f64));
  }
  auto const f64 = int_part / std::pow(10.0, -exponent);
  return NewToken(TokenData(f64));
}

Token* Lexer::NewIntLiteral(TokenType type, uint64_t u64) {
  return NewToken(TokenData(type, u64));
}

Token* Lexer::NewToken(TokenType type) {
  return NewToken(TokenData(type));
}

Token* Lexer::NewToken(const TokenData& data) {
  return session_->NewToken(ComputeLocation(), data);
}

base::char16 Lexer::PeekChar() {
  return input_stream_->PeekChar();
}

base::char16 Lexer::ReadChar() {
  ++token_end_;
  return input_stream_->ReadChar();
}

// Returns false when we don't get matching "*/" at end of source code.
// Note: Block comments is nestable.
bool Lexer::SkipBlockComment() {
  enum State {
    Asterisk,
    Normal,
    Slash,
  };
  auto state = Normal;
  auto depth = 1;
  while (!IsAtEndOfStream()) {
    Advance();
    auto const char_code = PeekChar();
    switch (state) {
      case State::Asterisk:
        if (char_code == '/') {
          --depth;
          if (!depth) {
            Advance();
            return true;
          }
          state = State::Normal;
          break;
        }
        if (char_code != '*')
          state = State::Normal;
        break;
      case State::Normal:
        if (char_code == '*') {
          state = State::Asterisk;
          break;
        }
        if (char_code == '/') {
          state = State::Slash;
          break;
        }
        break;
      case State::Slash:
        if (char_code == '*') {
          ++depth;
          state = State::Normal;
          break;
        }
        if (char_code != '/')
          state = State::Normal;
        break;
    }
  }
  return false;
}

// Note: Skip until unescaped newline or end of source code.
void Lexer::SkipLineComment() {
  enum State { Backslash, Normal, } state = Normal;
  while (!IsAtEndOfStream()) {
    Advance();
    auto const char_code = PeekChar();
    switch (state) {
      case State::Backslash:
        state = State::Normal;
        break;
      case State::Normal:
        if (char_code == '\n')
          return;
        if (char_code == '\\')
          state = State::Backslash;
        break;
    }
  }
}

}  // namespace compiler
}  // namespace elang
