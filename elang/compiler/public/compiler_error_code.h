// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_PUBLIC_COMPILER_ERROR_CODE_H_
#define ELANG_COMPILER_PUBLIC_COMPILER_ERROR_CODE_H_

namespace elang {
namespace compiler {

#define IGNORE_COMPILER_ERROR(category, subcategory, name)

#define FOR_EACH_COMPILER_ERROR_CODE(E, W)            \
  /* * Name Resolver */                               \
  E(NameResolution, Alias, NeitherNamespaceNorType)   \
  E(NameResolution, Class, Containing)                \
  E(NameResolution, Import, NeitherNamespaceNorType)  \
  E(NameResolution, Name, Ambiguous)                  \
  E(NameResolution, Name, Cycle)                      \
  E(NameResolution, Name, NeitherClassNortInterface)  \
  E(NameResolution, Name, NeitherNamespaceNorType)    \
  E(NameResolution, Name, NotClass)                   \
  E(NameResolution, Name, NotFound)                   \
  E(NameResolution, Name, NotInterface)               \
  E(NameResolution, Name, NotNamespace)               \
  E(NameResolution, Name, NotResolved)                \
  /*  Parser */                                       \
  /* CompilationUnit */                               \
  E(Syntax, CompilationUnit, Invalid)                 \
  /* EnumDecl */                                      \
  E(Syntax, EnumDecl, Expression)                     \
  E(Syntax, EnumDecl, LeftCurryBracket)               \
  E(Syntax, EnumDecl, Modifier)                       \
  E(Syntax, EnumDecl, NameDuplicate)                  \
  E(Syntax, EnumDecl, NameInvalid)                    \
  E(Syntax, EnumDecl, RightCurryBracket)              \
  /* ClassDecl */                                     \
  E(Syntax, ClassDecl, LeftCurryBracket)              \
  E(Syntax, ClassDecl, Modifier)                      \
  /* Expect name for class. */                        \
  E(Syntax, ClassDecl, Name)                          \
  /* Class name is already defined. */                \
  E(Syntax, ClassDecl, NameDuplicate)                 \
  E(Syntax, ClassDecl, RightCurryBracket)             \
  /* Expect ";" for |extern| class. */                \
  E(Syntax, ClassDecl, SemiColon)                     \
  E(Syntax, ClassDecl, TypeParamInvalid)              \
  /* ClassMember */                                   \
  E(Syntax, ClassMember, Duplicate)                   \
  E(Syntax, ClassMember, Name)                        \
  E(Syntax, ClassMember, Parenthesis)                 \
  E(Syntax, ClassMember, SemiColon)                   \
  E(Syntax, ClassMember, VarField)                    \
  /* Break */                                         \
  E(Syntax, Break, Invalid)                           \
  E(Syntax, Break, SemiColon)                         \
  /* Catch */                                         \
  E(Syntax, Catch, LeftCurryBracket)                  \
  E(Syntax, Catch, LeftParenthesis)                   \
  E(Syntax, Catch, RightParenthesis)                  \
  /* Continue */                                      \
  E(Syntax, Continue, Invalid)                        \
  E(Syntax, Continue, SemiColon)                      \
  /* Do */                                            \
  E(Syntax, Do, LeftParenthesis)                      \
  E(Syntax, Do, RightParenthesis)                     \
  E(Syntax, Do, SemiColon)                            \
  E(Syntax, Do, While)                                \
  /* Expression */                                    \
  E(Syntax, Expression, ConditionalColon)             \
  E(Syntax, Expression, ConditionalElse)              \
  E(Syntax, Expression, ConditionalThen)              \
  E(Syntax, Expression, RightParenthesis)             \
  E(Syntax, Expression, Type)                         \
  /* Finally */                                       \
  E(Syntax, Finally, LeftCurryBracket)                \
  /* For */                                           \
  E(Syntax, For, Colon)                               \
  E(Syntax, For, Init)                                \
  E(Syntax, For, LeftParenthesis)                     \
  E(Syntax, For, RightParenthesis)                    \
  E(Syntax, For, SemiColon)                           \
  E(Syntax, For, Var)                                 \
  /* If */                                            \
  E(Syntax, If, LeftParenthesis)                      \
  E(Syntax, If, RightParenthesis)                     \
  /* MemberAccess */                                  \
  E(Syntax, MemberAccess, Name)                       \
  E(Syntax, MemberAccess, RightAngleBracket)          \
  E(Syntax, MemberAccess, TypeArgument)               \
  /* Method */                                        \
  E(Syntax, Method, Comma)                            \
  E(Syntax, Method, LeftCurryBracket)                 \
  E(Syntax, Method, NameDuplicate)                    \
  E(Syntax, Method, RightCurryBracket)                \
  /* ';' without |extern| modifier. */                \
  E(Syntax, Method, SemiColon)                        \
  /* Modifier */                                      \
  E(Syntax, Modifier, Duplicate)                      \
  /* 'partial' modifier must be the last modifier. */ \
  E(Syntax, Modifier, Partial)                        \
  /* NamespaceDecl */                                 \
  E(Syntax, NamespaceDecl, LeftCurryBracket)          \
  E(Syntax, NamespaceDecl, NameDuplicate)             \
  E(Syntax, NamespaceDecl, RightCurryBracket)         \
  /* Return */                                        \
  E(Syntax, Return, SemiColon)                        \
  /* Statement */                                     \
  E(Syntax, Statement, SemiColon)                     \
  /* Throw */                                         \
  E(Syntax, Throw, Invalid)                           \
  E(Syntax, Throw, SemiColon)                         \
  /* Try */                                           \
  E(Syntax, Try, LeftCurryBracket)                    \
  /* Type */                                          \
  E(Syntax, Type, Comma)                              \
  E(Syntax, Type, DotNotName)                         \
  E(Syntax, Type, Name)                               \
  E(Syntax, Type, RightAngleBracket)                  \
  E(Syntax, Type, RightSquareBracket)                 \
  E(Syntax, Type, TypeArgument)                       \
  /* Using */                                         \
  E(Syntax, Using, Assign)                            \
  E(Syntax, Using, LeftParenthesis)                   \
  E(Syntax, Using, RightParenthesis)                  \
  E(Syntax, Using, Name)                              \
  /* Using directive*/                                \
  E(Syntax, UsingDirective, Alias)                    \
  E(Syntax, UsingDirective, Duplicate)                \
  E(Syntax, UsingDirective, Import)                   \
  E(Syntax, UsingDirective, Name)                     \
  E(Syntax, UsingDirective, SemiColon)                \
  /* Var */                                           \
  E(Syntax, Var, Assign)                              \
  E(Syntax, Var, Comma)                               \
  E(Syntax, Var, Const)                               \
  E(Syntax, Var, Name)                                \
  E(Syntax, Var, SemiColon)                           \
  E(Syntax, Var, Type)                                \
  /* While */                                         \
  E(Syntax, While, LeftParenthesis)                   \
  E(Syntax, While, RightParenthesis)                  \
  /* Lexer */                                         \
  E(Token, AtMark, Invalid)                           \
  E(Token, AtMarkString, Unclosed)                    \
  E(Token, Backslash, Invalid)                        \
  E(Token, BackslashU, Invalid)                       \
  E(Token, BlockComment, Unclosed)                    \
  E(Token, Character, Invalid)                        \
  E(Token, Float, ExponentOverflow)                   \
  E(Token, Integer, Invalid)                          \
  E(Token, Integer, Overflow)                         \
  E(Token, Real, TooManyDigits)                       \
  E(Token, String, HasNewline)                        \
  E(Token, String, Unclosed)

//////////////////////////////////////////////////////////////////////
//
// ErrorCode
//
enum class ErrorCode {
#define E(category, subcategory, name) category##subcategory##name,
  FOR_EACH_COMPILER_ERROR_CODE(E, E)
#undef E
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_PUBLIC_COMPILER_ERROR_CODE_H_
