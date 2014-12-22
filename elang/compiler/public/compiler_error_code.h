// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(elang_compiler_public_compiler_error_code_h)
#define elang_compiler_public_compiler_error_code_h

namespace elang {
namespace compiler {

#define IGNORE_COMPILER_ERROR(category, name)

#define COMPILER_ERROR_CODE_LIST(E, W) \
  E(Lexer, BadAtMark) \
  E(Lexer, BadCharLiteral) \
  E(Lexer, BadInteger) \
  E(Lexer, ExponentOverflow) \
  E(Lexer, IntegerOverflow) \
  E(Lexer, InvalidBackslash) \
  E(Lexer, InvalidBackslashU) \
  E(Lexer, NewlineInStringLiteral) \
  E(Lexer, TooManyDigits) \
  E(Lexer, UnterminatedBlockComment) \
  E(Lexer, UnterminatedString)

//////////////////////////////////////////////////////////////////////
//
// ErrorCode
//
enum class ErrorCode {
  #define E(category, name) category ## name,
  COMPILER_ERROR_CODE_LIST(E, E)
  #undef E
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(elang_compiler_public_compiler_error_code_h)

