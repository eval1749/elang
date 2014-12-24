// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_token_type_h)
#define INCLUDE_elang_compiler_token_type_h

#if _DEBUG
#include <ostream>
#endif

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
        K(Abstract, "abstract", "KS") \
    /* B */ \
        K(Break, "break", "KS") \
    /* C */ \
        K(Case, "case", "KS") \
        K(Catch, "catch", "KS") \
        K(Class, "class", "KS") \
        K(Const, "const", "KS") \
        K(Continue, "continue", "KS") \
    /* D */ \
        K(Default, "default", "KS") \
        K(Do, "do", "KS") \
    /* E */ \
        K(Else, "else", "KS") \
        K(Enum, "enum", "KS") \
        K(Explicit, "explicit", "KS") \
        K(Extends, "extends", "KS") \
        K(Extern, "extern", "KS") \
    /* F */ \
        K(Final, "final", "KS") \
        K(Finally, "finally", "KS") \
        K(For, "for", "KS") \
        K(Function, "function", "KS") \
    /* G */ \
        K(Goto, "goto", "KS") \
    /* I */ \
        K(If, "if", "KS") \
        K(Implicit, "implicit", "CS") \
        K(Implements, "implements", "CS") \
        K(Interface, "interface", "KS") \
    /* N */ \
        K(Namespace, "namespace", "KS") \
        K(New, "new", "KS") \
    /* O */ \
        K(Override, "override", "KS") \
    /* P */ \
        K(Private, "private", "KS") \
        K(Protected, "protected", "KS") \
        K(Public, "public", "KS") \
    /* R */ \
        K(Return, "return", "KS") \
    /* S */ \
        K(SizeOf, "sizeof", "KS") \
        K(Static, "static", "KS") \
        K(Struct, "struct", "KS") \
        K(Super, "super", "KS") \
        K(Switch, "switch", "KS") \
    /* T */ \
        K(This, "this", "KS") \
        K(Throw, "throw", "KS") \
        K(Try, "try", "KS") \
        K(TypeOf, "typeof", "KS") \
    /* U */ \
        K(Using, "using", "KS") \
    /* V */ \
        K(Var, "var", "KS") \
        K(Virtual, "virtual", "KS") \
        K(Void, "void", "KS") \
    K(Volatile, "volatile", "KS") \
    /* W */ \
        K(Where, "where", "CS") \
        K(While, "while", "KS") \
    /* Y */ \
        K(Yield, "yield", "KS") \
    /* known types */ \
    K(Bool, "bool", "KS") \
    K(Float32, "float32", "KS") \
    K(Float64, "float64", "KS") \
    K(Int8, "int8", "KS") \
    K(Int16, "int16", "KS") \
    K(Int32, "int32", "KS") \
    K(Int64, "int64", "KS") \
    K(UInt8, "uint8", "KS") \
    K(UInt16, "uint16", "KS") \
    K(UInt32, "uint32", "KS") \
    K(UInt64, "uint64", "KS") \
    /* literals */ \
    K(NullLiteral, "null", "KS") \
    K(TrueLiteral, "true", "KS") \
    K(FalseLiteral, "false", "KS") \
    T(Float32Literal, "f32", "LF") \
    T(Float64Literal, "f64", "LF") \
    T(Int32Literal, "I32", "LI") \
    T(Int64Literal, "I64", "LI") \
    T(UInt32Literal, "U32", "LU") \
    T(UInt64Literal, "U64", "LU") \
    T(CharacterLiteral, "'c'", "LC") \
    T(StringLiteral, "\"string\"", "LS") \
\
    T(SimpleName, "SimpleName", "NS") \
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

#if _DEBUG
std::ostream& operator<<(std::ostream& ostream, TokenType token_type);
#endif

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_token_type_h)

