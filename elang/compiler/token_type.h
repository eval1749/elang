// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_token_type_h)
#define INCLUDE_elang_compiler_token_type_h

#include <ostream>

namespace elang {
namespace compiler {

// IGNORE_TOKEN is a convenience macro that can be supplied as
// an argument (at any position) for a TOKEN_LIST call. It does
// nothing with tokens belonging to the respective category.
#define IGNORE_TOKEN(name, string, details)

// First character of details ::=
//  '?' special
//  'C' contextual keyword
//  'L' literal
//  'K' keyword
//  'N' name
//  'O' binary operator followed by precedence from 'a' highest to 'm' lowest.
//  'P' punctuation
//  'U' unary operator
// Second character of details ::=
//  'C' character data
//  'F' float data
//  'I' integer data
//  'N' simple name
//  'S' string data
//  'a' operator precedence (lowest)
//  ...
//  'm' operator precedence (highest)
#define TOKEN_LIST(T, K) \
    T(EndOfSource, "EOS", "?") \
    T(None, "None", "?") \
\
    T(Colon, ":", "P") \
    T(SemiColon, ";", "P") \
    T(LeftParenthesis, ")", "P") \
    T(RightParenthesis, ")", "P") \
    T(LeftSquareBracket, "]", "P") \
    T(RightSquareBracket, "]", "P") \
    T(LeftCurryBracket, "{", "P") \
    T(RightCurryBracket, "}", "P") \
    /* Appeared just after name, e.g. |GenericType<|, |Type?|. */ \
    T(LeftAngleBracket, "<", "P") \
    T(OptionalType, "?", "P") \
    T(Arrow, "=>", "P") \
\
    /* Operators */ \
    T(Comma, ",", "Oa") \
    T(Assign, "=", "Ob") \
    T(BitOrAssign, "|=", "Ob") \
    T(BitAndAssign, "&=", "Ob") \
    T(BitXorAssign, "&=", "Ob") \
    T(ShiftLeftAssign, "<<=", "Ob") \
    T(ShiftRightAssign, ">>=", "Ob") \
    T(AddAssign, "+=", "Ob") \
    T(SubAssign, "-=", "Ob") \
    T(MulAssign, "*=", "Ob") \
    T(DivAssign, "/=", "Ob") \
    T(ModAssign, "%=", "Ob") \
    T(ShlAssign, "<<=", "Ob") \
    T(ShrAssign, ">>=", "Ob") \
    /* tenary operations */ \
    T(QuestionMark, "?", "Oc") \
    T(NullOr, "??", "Od") \
    /* logical operations */ \
    T(Or, "||", "Od") \
    T(And, "&&", "Oe") \
    /* bit operations */ \
    T(BitOr, "|", "Of") \
    T(BitXor, "^", "Og") \
    T(BitAnd, "&", "Oh") \
    /* equality */ \
    T(Eq, "==", "Oi") \
    T(Ne, "!=", "Oi") \
    /* comparison */ \
    T(Lt, "<", "Oj") \
    T(Le, "<=", "Oj") \
    T(Gt, ">", "Oj") \
    T(Ge, ">=", "Oj") \
    K(as, "as", "Oj") \
    T(Shl, "<<", "Ok") \
    T(Shr, ">>", "Ok") \
    T(Add, "+", "Ol") \
    T(Sub, "-", "Ol") \
    T(Mul, "*", "Om") \
    T(Div, "/", "Om") \
    T(Mod, "%", "Om") \
    /* Unary operators */ \
    T(Decrement, "--", "U") \
    T(Increment, "++", "U") \
    T(Dot, ".", "P") \
    T(Not, "!", "U") \
    T(BitNot, "~", "U") \
    /* Keywords */ \
    /* A */ \
        K(Abstract, "abstract", "KN") \
    /* B */ \
        K(Break, "break", "KN") \
    /* C */ \
        K(Case, "case", "KN") \
        K(Catch, "catch", "KN") \
        K(Class, "class", "KN") \
        K(Const, "const", "KN") \
        K(Continue, "continue", "KN") \
    /* D */ \
        K(Default, "default", "KN") \
        K(Do, "do", "KN") \
    /* E */ \
        K(Else, "else", "KN") \
        K(Enum, "enum", "KN") \
        K(Explicit, "explicit", "KN") \
        K(Extends, "extends", "KN") \
        K(Extern, "extern", "KN") \
    /* F */ \
        K(Final, "final", "KN") \
        K(Finally, "finally", "KN") \
        K(For, "for", "KN") \
        K(Function, "function", "KN") \
    /* G */ \
        K(Goto, "goto", "KN") \
    /* I */ \
        K(If, "if", "KN") \
        K(Implicit, "implicit", "CS") \
        K(Implements, "implements", "CS") \
        K(Interface, "interface", "KN") \
    /* N */ \
        K(Namespace, "namespace", "KN") \
        K(New, "new", "KN") \
    /* O */ \
        K(Override, "override", "KN") \
    /* P */ \
        K(Private, "private", "KN") \
        K(Protected, "protected", "KN") \
        K(Public, "public", "KN") \
    /* R */ \
        K(Return, "return", "KN") \
    /* S */ \
        K(SizeOf, "sizeof", "KN") \
        K(Static, "static", "KN") \
        K(Struct, "struct", "KN") \
        K(Super, "super", "KN") \
        K(Switch, "switch", "KN") \
    /* T */ \
        K(This, "this", "KN") \
        K(Throw, "throw", "KN") \
        K(Try, "try", "KN") \
        K(TypeOf, "typeof", "KN") \
    /* U */ \
        K(Using, "using", "KN") \
    /* V */ \
        K(Var, "var", "KN") \
        K(Virtual, "virtual", "KN") \
        K(Void, "void", "KN") \
    K(Volatile, "volatile", "KN") \
    /* W */ \
        K(Where, "where", "CS") \
        K(While, "while", "KN") \
    /* Y */ \
        K(Yield, "yield", "KN") \
    /* known types */ \
    K(Bool, "bool", "KN") \
    K(Float32, "float32", "KN") \
    K(Float64, "float64", "KN") \
    K(Int8, "int8", "KN") \
    K(Int16, "int16", "KN") \
    K(Int32, "int32", "KN") \
    K(Int64, "int64", "KN") \
    K(UInt8, "uint8", "KN") \
    K(UInt16, "uint16", "KN") \
    K(UInt32, "uint32", "KN") \
    K(UInt64, "uint64", "KN") \
    /* literals */ \
    K(NullLiteral, "null", "KN") \
    K(TrueLiteral, "true", "KN") \
    K(FalseLiteral, "false", "KN") \
    T(Float32Literal, "f32", "LF") \
    T(Float64Literal, "f64", "LF") \
    T(Int32Literal, "I32", "LI") \
    T(Int64Literal, "I64", "LI") \
    T(UInt32Literal, "U32", "LU") \
    T(UInt64Literal, "U64", "LU") \
    T(CharacterLiteral, "'c'", "LC") \
    T(StringLiteral, "\"string\"", "LS") \
\
    T(SimpleName, "SimpleName", "NN") \
    T(Illegal, "ILLIEGAL", "?")

//////////////////////////////////////////////////////////////////////
//
// TokenType
//
enum class TokenType {
  #define T(name, string, precedence) name,
  TOKEN_LIST(T, T)
  #undef T
};

std::ostream& operator<<(std::ostream& ostream, TokenType token_type);

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_token_type_h)

