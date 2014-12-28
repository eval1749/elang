// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_public_compiler_error_code_h)
#define INCLUDE_elang_compiler_public_compiler_error_code_h

namespace elang {
namespace compiler {

#define IGNORE_COMPILER_ERROR(category, subcategory, name)

#define COMPILER_ERROR_CODE_LIST(E, W) \
  /*  Parser Expression */ \
    E(Expression, Conditional, Colon) \
    E(Expression, Primary, RightParenthesis) \
  /* * Name Resolver */ \
    E(NameResolution, Alias, NoTarget) \
    E(NameResolution, Class, Containing) \
    E(NameResolution, Name, Ambiguous) \
    E(NameResolution, Name, Cycle) \
    E(NameResolution, Name, NeitherClassNortInterface) \
    E(NameResolution, Name, NeitherNamespaceNorType) \
    E(NameResolution, Name, NotClass) \
    E(NameResolution, Name, NotFound) \
    E(NameResolution, Name, NotInterface) \
    E(NameResolution, Name, NotNamespace) \
  /*  Parser */ \
    /* AliasDef */ \
      E(Syntax, AliasDef, AliasName) \
      E(Syntax, AliasDef, RealName) \
    /* EnumDecl */ \
      E(Syntax, EnumDecl, Expression) \
      E(Syntax, EnumDecl, LeftCurryBracket) \
      E(Syntax, EnumDecl, Modifier) \
      E(Syntax, EnumDecl, NameDuplicate) \
      E(Syntax, EnumDecl, NameInvalid) \
      E(Syntax, EnumDecl, RightCurryBracket) \
    /* ClassDecl */ \
      E(Syntax, ClassDecl, LeftCurryBracket) \
      E(Syntax, ClassDecl, Modifier) \
      /* Expect name for class. */ \
      E(Syntax, ClassDecl, Name) \
      /* Class name is already defined. */ \
      E(Syntax, ClassDecl, NameDuplicate) \
      E(Syntax, ClassDecl, RightCurryBracket) \
      /* Expect ";" for |extern| class. */ \
      E(Syntax, ClassDecl, SemiColon) \
      E(Syntax, ClassDecl, TypeParamInvalid) \
    /* ClassMember */ \
      E(Syntax, ClassMember, Duplicate) \
      E(Syntax, ClassMember, Name) \
      E(Syntax, ClassMember, Parenthesis) \
      E(Syntax, ClassMember, SemiColon) \
      E(Syntax, ClassMember, VarField) \
    /* Method */ \
      E(Syntax, Method, Comma) \
      E(Syntax, Method, LeftCurryBracket) \
      E(Syntax, Method, NameDuplicate) \
      /* ';' without |extern| modifier. */ \
      E(Syntax, Method, SemiColon) \
    /* Modifier */ \
      E(Syntax, Modifier, Duplicate) \
      /* 'partial' modifier must be the last modifier. */ \
      E(Syntax, Modifier, Partial) \
    /* NamespaceDecl */ \
      E(Syntax, NamespaceDecl, LeftCurryBracket) \
      E(Syntax, NamespaceDecl, NameDuplicate) \
      E(Syntax, NamespaceDecl, RightCurryBracket) \
    /* Type */ \
      E(Syntax, Type, Comma) \
      E(Syntax, Type, DotNotName) \
      E(Syntax, Type, RightSquareBracket) \
    /* UsingDirective */ \
      E(Syntax, UsingDirective, Invalid) \
      E(Syntax, UsingDirective, Name) \
      E(Syntax, UsingDirective, SemiColon) \
  /* Lexer */ \
    E(Token, AtMark, Invalid) \
    E(Token, AtMarkString, Unclosed) \
    E(Token, Backslash, Invalid) \
    E(Token, BackslashU, Invalid) \
    E(Token, BlockComment, Unclosed) \
    E(Token, Character, Invalid) \
    E(Token, Float, ExponentOverflow) \
    E(Token, Integer, Invalid) \
    E(Token, Integer, Overflow) \
    E(Token, Real, TooManyDigits) \
    E(Token, String, HasNewline) \
    E(Token, String, Unclosed)

//////////////////////////////////////////////////////////////////////
//
// ErrorCode
//
enum class ErrorCode {
  #define E(category, subcategory, name) category ## subcategory ## name,
  COMPILER_ERROR_CODE_LIST(E, E)
  #undef E
};

}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_public_compiler_error_code_h)

