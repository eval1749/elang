// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_PUBLIC_COMPILER_ERROR_CODE_H_
#define ELANG_COMPILER_PUBLIC_COMPILER_ERROR_CODE_H_

namespace elang {
namespace compiler {

#define IGNORE_COMPILER_ERROR(category, subcategory, name)

#define FOR_EACH_COMPILER_ERROR_CODE(E, W)            \
  E(Error, Code, Zero)                                \
  /* Analyzer */                                      \
  E(Analyze, Expression, NotConstant)                 \
  E(Analyze, Expression, Type)                        \
  E(Analyze, Type, NotFound)                          \
  E(Analyze, Type, NotType)                           \
  /* Class Analyzer */                                \
  E(ClassResolution, Method, Conflict)                \
  E(ClassResolution, Method, Duplicate)               \
  /* Code Generator */                                \
  E(CodeGenerator, Expression, NotYetImplemented)     \
  E(CodeGenerator, Return, None)                      \
  E(CodeGenerator, Statement, NotYetImplemented)      \
  /* Method Analyzer */                               \
  E(Method, Callee, NotSupported)                     \
  E(Method, Return, NotVoid)                          \
  E(Method, Return, Void)                             \
  /* Namespace Analyzer */                            \
  E(NameResolution, Alias, Duplicate)                 \
  E(NameResolution, Alias, NeitherNamespaceNorType)   \
  E(NameResolution, Class, Conflict)                  \
  E(NameResolution, Class, Containing)                \
  E(NameResolution, Class, Duplicate)                 \
  E(NameResolution, Class, Modifiers)                 \
  E(NameResolution, Class, NotClass)                  \
  E(NameResolution, Class, NotResolved)               \
  E(NameResolution, Class, Partial)                   \
  E(NameResolution, Import, NeitherNamespaceNorType)  \
  E(NameResolution, MemberAccess, NotFound)           \
  E(NameResolution, Name, Ambiguous)                  \
  E(NameResolution, Name, Cycle)                      \
  E(NameResolution, Name, NeitherClassNorInterface)   \
  E(NameResolution, Name, NeitherNamespaceNorType)    \
  E(NameResolution, Name, NotClass)                   \
  E(NameResolution, Name, NotFound)                   \
  E(NameResolution, Name, NotInterface)               \
  E(NameResolution, Name, NotNamespace)               \
  E(NameResolution, Name, NotResolved)                \
  E(NameResolution, SystemObject, HasBaseClass)       \
  /*  Predefined names */                             \
  E(PredefinedNames, Name, NotClass)                  \
  E(PredefinedNames, Name, NotFound)                  \
  E(PredefinedNames, Name, NotNamespace)              \
  /* Analyzer */                                      \
  E(Semantic, Enum, EnumBase)                         \
  E(Semantic, Enum, MemberValue)                      \
  E(Semantic, IntAdd, Overflow)                       \
  E(Semantic, UIntAdd, Overflow)                      \
  E(Semantic, Value, Type)                            \
  /*  Parser */                                       \
  E(Syntax, Bracket, Extra)                           \
  E(Syntax, Bracket, NotClosed)                       \
  /* CompilationUnit */                               \
  E(Syntax, CompilationUnit, Invalid)                 \
  /* ClassDecl */                                     \
  E(Syntax, Class, Conflict)                          \
  /* Class name is already defined. */                \
  E(Syntax, Class, Duplicate)                         \
  E(Syntax, Class, LeftCurryBracket)                  \
  E(Syntax, Class, Modifier)                          \
  /* Expect name for class. */                        \
  E(Syntax, Class, Name)                              \
  E(Syntax, Class, Partial)                           \
  E(Syntax, Class, PartialModifiers)                  \
  E(Syntax, Class, RightCurryBracket)                 \
  /* Expect ";" for |extern| class. */                \
  E(Syntax, Class, SemiColon)                         \
  E(Syntax, Class, TypeParamInvalid)                  \
  /* ClassMember */                                   \
  E(Syntax, ClassMember, Conflict)                    \
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
  /* EnumDecl */                                      \
  E(Syntax, Enum, Conflict)                           \
  E(Syntax, Enum, Duplicate)                          \
  E(Syntax, Enum, Expression)                         \
  E(Syntax, Enum, LeftCurryBracket)                   \
  E(Syntax, Enum, Modifier)                           \
  E(Syntax, Enum, NameInvalid)                        \
  E(Syntax, Enum, RightCurryBracket)                  \
  /* Expression */                                    \
  E(Syntax, Expression, ArrayAccess)                  \
  E(Syntax, Expression, Call)                         \
  E(Syntax, Expression, ConditionalColon)             \
  E(Syntax, Expression, ConditionalElse)              \
  E(Syntax, Expression, ConditionalThen)              \
  E(Syntax, Expression, Label)                        \
  E(Syntax, Expression, LeftAngleBracket)             \
  E(Syntax, Expression, RightParenthesis)             \
  E(Syntax, Expression, RightSquareBracket)           \
  E(Syntax, Expression, Type)                         \
  E(Syntax, Expression, UnboundVariable)              \
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
  E(Syntax, Method, Body)                             \
  E(Syntax, Method, Comma)                            \
  E(Syntax, Method, LeftCurryBracket)                 \
  E(Syntax, Method, NameDuplicate)                    \
  E(Syntax, Method, Parameter)                        \
  E(Syntax, Method, RightCurryBracket)                \
  /* ';' without |extern| modifier. */                \
  E(Syntax, Method, SemiColon)                        \
  /* Modifier */                                      \
  E(Syntax, Modifier, Duplicate)                      \
  /* 'partial' modifier must be the last modifier. */ \
  E(Syntax, Modifier, Partial)                        \
  /* Namespace */                                     \
  E(Syntax, Namespace, Anonymous)                     \
  E(Syntax, Namespace, Conflict)                      \
  E(Syntax, Namespace, Invalid)                       \
  E(Syntax, Namespace, Name)                          \
  E(Syntax, Namespace, LeftCurryBracket)              \
  E(Syntax, Namespace, RightCurryBracket)             \
  /* Return */                                        \
  E(Syntax, Return, SemiColon)                        \
  /* Statement */                                     \
  E(Syntax, Statement, SemiColon)                     \
  E(Syntax, Statement, Unreachable)                   \
  /* Throw */                                         \
  E(Syntax, Throw, Invalid)                           \
  E(Syntax, Throw, SemiColon)                         \
  /* Try */                                           \
  E(Syntax, Try, LeftCurryBracket)                    \
  /* Type */                                          \
  E(Syntax, Type, Comma)                              \
  E(Syntax, Type, Name)                               \
  E(Syntax, Type, NotType)                            \
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
  E(Syntax, Var, Duplicate)                           \
  E(Syntax, Var, Initializer)                         \
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
  E(Token, String, Unclosed)                          \
  /* Translator */                                    \
  E(Translator, Expression, NotYetImplemented)        \
  E(Translator, Expression, Unexpected)               \
  E(Translator, Return, None)                         \
  E(Translator, Statement, NotYetImplemented)         \
  /* Type Resolver */                                 \
  E(TypeResolver, ArrayAccess, Array)                 \
  E(TypeResolver, ArrayAccess, Index)                 \
  E(TypeResolver, ArrayAccess, Rank)                  \
  E(TypeResolver, Argument, Unify)                    \
  E(TypeResolver, Assignment, LeftValue)              \
  E(TypeResolver, BinaryOperation, Equality)          \
  E(TypeResolver, BinaryOperation, Numeric)           \
  E(TypeResolver, BinaryOperation, Shift)             \
  E(TypeResolver, Callee, NotSupported)               \
  E(TypeResolver, Conditional, NotMatch)              \
  E(TypeResolver, Expression, Invalid)                \
  E(TypeResolver, Expression, NotBool)                \
  E(TypeResolver, Expression, NotYetImplemented)      \
  E(TypeResolver, IncrementExpression, Type)          \
  E(TypeResolver, IncrementExpression, Place)         \
  E(TypeResolver, ForEach, ElementType)               \
  E(TypeResolver, Method, Ambiguous)                  \
  E(TypeResolver, Method, NoMatch)                    \
  E(TypeResolver, Method, Return)                     \
  E(TypeResolver, Statement, NotYetImplemented)       \
  E(TypeResolver, UnaryOperation, Type)               \
  E(TypeResolver, Variable, NotResolved)              \
  /* Warnings */                                      \
  W(Warning, Code, Zero)                              \
  W(Syntax, Var, NotUsed)

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
