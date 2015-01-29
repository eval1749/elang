// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TOKEN_TYPE_H_
#define ELANG_COMPILER_TOKEN_TYPE_H_

#include <string>
#include <ostream>

namespace elang {
namespace compiler {

// IGNORE_TOKEN is a convenience macro that can be supplied as
// an argument (at any position) for a FOR_EACH_TOKEN call. It does
// nothing with tokens belonging to the respective category.
#define IGNORE_TOKEN(name, string, details)

// First character of details ::=
//  '?' special
//  'C' contextual keyword
//  'L' literal
//  'K' keyword
//  'N' simple name
//  'O' binary operator followed by precedence from 'a' highest to 'm' lowest.
//  'P' punctuation
//  'U' unary operator
// Second character of details ::=
//  'C' character data
//  'F' float data
//  'I' integer data
//  'S' string data
//
// Second characters are
//  'L' keyword literal, e.g. false, null, true.
//  'L' left bracket
//  'M' modifier
//  'R' right bracket
//  'T' type
//  'a' operator precedence (highest)
//  ...
//  'm' operator precedence (lowest)
#define FOR_EACH_TOKEN(T, K)                                                \
  T(EndOfSource, "EOS", "?")                                                \
                                                                            \
  T(Colon, ":", "P")                                                        \
  T(SemiColon, ";", "P")                                                    \
  T(LeftParenthesis, "(", "PL")                                             \
  T(RightParenthesis, ")", "PR")                                            \
  T(LeftSquareBracket, "[", "PL")                                           \
  T(RightSquareBracket, "]", "PR")                                          \
  T(LeftCurryBracket, "{", "PL")                                            \
  T(RightCurryBracket, "}", "PR")                                           \
  /* For type arguments |G<T>| */                                           \
  T(LeftAngleBracket, "<", "PL")                                            \
  T(RightAngleBracket, ">", "PR")                                           \
  /* For nullable type, e.g. |int?| */                                      \
  T(OptionalType, "?", "P")                                                 \
  T(Dot, ".", "P")                                                          \
  T(OptionalDot, "?.", "P")                                                 \
  T(Comma, ",", "P")                                                        \
  T(Arrow, "=>", "P")                                                       \
                                                                            \
  /* Operators */                                                           \
  T(Assign, "=", "On")                                                      \
  T(BitOrAssign, "|=", "On")                                                \
  T(BitAndAssign, "&=", "On")                                               \
  T(BitXorAssign, "&=", "On")                                               \
  T(ShiftLeftAssign, "<<=", "On")                                           \
  T(ShiftRightAssign, ">>=", "On")                                          \
  T(AddAssign, "+=", "On")                                                  \
  T(SubAssign, "-=", "On")                                                  \
  T(MulAssign, "*=", "On")                                                  \
  T(DivAssign, "/=", "On")                                                  \
  T(ModAssign, "%=", "On")                                                  \
  T(ShlAssign, "<<=", "On")                                                 \
  T(ShrAssign, ">>=", "On")                                                 \
  /* tenary operations */                                                   \
  T(QuestionMark, "?", "Om")                                                \
  T(NullOr, "??", "Ol")                                                     \
  T(Or, "||", "Ok")                                                         \
  T(And, "&&", "Oj")                                                        \
  T(BitOr, "|", "Oi")                                                       \
  T(BitXor, "^", "Oh")                                                      \
  T(BitAnd, "&", "Og")                                                      \
  /* '<' and '>' are recognized just after whitespace. */                   \
  T(Lt, "<", "Of")                                                          \
  T(Le, "<=", "Of")                                                         \
  T(Gt, ">", "Of")                                                          \
  T(Ge, ">=", "Of")                                                         \
  T(Eq, "==", "Oe")                                                         \
  T(Ne, "!=", "Oe")                                                         \
  T(Shl, "<<", "Od")                                                        \
  T(Shr, ">>", "Od")                                                        \
  T(Add, "+", "Oc")                                                         \
  T(Sub, "-", "Oc")                                                         \
  T(Mul, "*", "Ob")                                                         \
  T(Div, "/", "Ob")                                                         \
  T(Mod, "%", "Ob")                                                         \
  /* Unary operators */                                                     \
  T(Decrement, "--", "Oa")                                                  \
  T(Increment, "++", "Oa")                                                  \
  T(Not, "!", "Oa")                                                         \
  T(BitNot, "~", "Oa")                                                      \
  /* |PostDecrement| and |PostIncrment| tokens are produced by parser */    \
  /* rather than lexer. */                                                  \
  T(PostDecrement, "--", "P")                                               \
  T(PostIncrement, "++", "P")                                               \
  /* |UnaryAdd| and |UnarySub| tokens are produced by parser rather than */ \
  /* lexer. */                                                              \
  T(UnaryAdd, "+", "+")                                                     \
  T(UnarySub, "-", "-")                                                     \
  /* Keywords */                                                            \
  /* A */                                                                   \
  K(Abstract, "abstract", "KM")                                             \
  /* B */                                                                   \
  K(Break, "break", "K-")                                                   \
  /* C */                                                                   \
  K(Case, "case", "K-")                                                     \
  K(Catch, "catch", "K-")                                                   \
  K(Class, "class", "K-")                                                   \
  K(Const, "const", "K-")                                                   \
  K(Continue, "continue", "K-")                                             \
  /* D */                                                                   \
  K(Default, "default", "K-")                                               \
  K(Do, "do", "K-")                                                         \
  K(DynamicCast, "dynamic_cast", "K-")                                      \
  /* E */                                                                   \
  K(Else, "else", "K-")                                                     \
  K(Enum, "enum", "K-")                                                     \
  K(Explicit, "explicit", "K-")                                             \
  K(Extern, "extern", "KM")                                                 \
  /* F */                                                                   \
  K(Final, "final", "KM")                                                   \
  K(Finally, "finally", "K-")                                               \
  K(For, "for", "K-")                                                       \
  K(Function, "function", "K-")                                             \
  /* G */                                                                   \
  K(Goto, "goto", "K-")                                                     \
  /* I */                                                                   \
  K(If, "if", "K-")                                                         \
  K(Implicit, "implicit", "C-")                                             \
  K(Interface, "interface", "K-")                                           \
  /* N */                                                                   \
  K(Namespace, "namespace", "K-")                                           \
  K(New, "new", "K-")                                                       \
  /* O */                                                                   \
  K(Operator, "operator", "K-")                                             \
  K(Override, "override", "KM")                                             \
  /* P */                                                                   \
  K(Partial, "partial", "KM")                                               \
  K(Private, "private", "KM")                                               \
  K(Protected, "protected", "KM")                                           \
  K(Public, "public", "KM")                                                 \
  /* R */                                                                   \
  K(Return, "return", "K-")                                                 \
  /* S */                                                                   \
  K(SizeOf, "sizeof", "K-")                                                 \
  K(Static, "static", "KM")                                                 \
  K(StaticCast, "static_cast", "K-")                                        \
  K(Struct, "struct", "K-")                                                 \
  K(Super, "super", "KP")                                                   \
  K(Switch, "switch", "K-")                                                 \
  /* T */                                                                   \
  K(This, "this", "KP")                                                     \
  K(Throw, "throw", "K-")                                                   \
  K(Try, "try", "K-")                                                       \
  K(TypeOf, "typeof", "K-")                                                 \
  /* U */                                                                   \
  K(Using, "using", "K-")                                                   \
  /* V */                                                                   \
  K(Var, "var", "K-")                                                       \
  K(Virtual, "virtual", "KM")                                               \
  K(Volatile, "volatile", "KM")                                             \
  /* W */                                                                   \
  K(Where, "where", "C-")                                                   \
  K(While, "while", "K-")                                                   \
  /* Y */                                                                   \
  K(Yield, "yield", "K-")                                                   \
  /* keyword types */                                                       \
  K(Bool, "bool", "KT")                                                     \
  K(Char, "char", "KT")                                                     \
  K(Float32, "float32", "KT")                                               \
  K(Float64, "float64", "KT")                                               \
  K(Int, "int", "KT") /* |int| is alias of |int32|. */                      \
  K(Int16, "int16", "KT")                                                   \
  K(Int32, "int32", "KT")                                                   \
  K(Int64, "int64", "KT")                                                   \
  K(Int8, "int8", "KT")                                                     \
  K(UInt16, "uint16", "KT")                                                 \
  K(UInt32, "uint32", "KT")                                                 \
  K(UInt64, "uint64", "KT")                                                 \
  K(UInt8, "uint8", "KT")                                                   \
  K(Void, "void", "KT")                                                     \
  /* literals */                                                            \
  K(NullLiteral, "null", "KL")                                              \
  /* typed literals */                                                      \
  T(CharacterLiteral, "'c'", "LC")                                          \
  K(FalseLiteral, "false", "KL")                                            \
  T(Float32Literal, "f32", "LF")                                            \
  T(Float64Literal, "f64", "LF")                                            \
  T(Int32Literal, "I32", "LI")                                              \
  T(Int64Literal, "I64", "LI")                                              \
  T(UInt32Literal, "U32", "LU")                                             \
  T(UInt64Literal, "U64", "LU")                                             \
  T(StringLiteral, "\"string\"", "LS")                                      \
  K(TrueLiteral, "true", "KL")                                              \
  /* names */                                                               \
  T(SimpleName, "SimpleName", "NN")                                         \
  T(TempName, "TempName", "NN")                                             \
  T(VerbatimName, "VerbatimName", "NN")                                     \
                                                                            \
  T(Illegal, "ILLIEGAL", "?")

//////////////////////////////////////////////////////////////////////
//
// TokenType
//
enum class TokenType {
#define T(name, string, precedence) name,
  FOR_EACH_TOKEN(T, T)
#undef T
};

// Order of this list must be synced with FOR_EACH_TOKEN.
#define FOR_EACH_TYPE_KEYWORD(V) \
  V(Bool)                        \
  V(Char)                        \
  V(Float32)                     \
  V(Float64)                     \
  V(Int)                         \
  V(Int16)                       \
  V(Int32)                       \
  V(Int64)                       \
  V(Int8)                        \
  V(UInt16)                      \
  V(UInt32)                      \
  V(UInt64)                      \
  V(UInt8)                       \
  V(Void)

std::ostream& operator<<(std::ostream& ostream, TokenType token_type);

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TOKEN_TYPE_H_
