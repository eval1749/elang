// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_public_compiler_error_code_h)
#define INCLUDE_elang_compiler_public_compiler_error_code_h

namespace elang {
namespace compiler {

#define IGNORE_COMPILER_ERROR(category, subcategory, name)

#define COMPILER_ERROR_CODE_LIST(E, W) \
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
  E(Token, String, Unclosed) \
  /* AliasDef */ \
  E(Syntax, AliasDef, AliasName) \
  E(Syntax, AliasDef, RealName) \
  /* EnumDecl */ \
  E(Syntax, EnumDecl, LeftCurryBracket) \
  E(Syntax, EnumDecl, Modifier) \
  E(Syntax, EnumDecl, NameDuplicate) \
  E(Syntax, EnumDecl, NameInvalid) \
  E(Syntax, EnumDecl, RightCurryBracket) \
  /* FieldDecl */ \
  E(Syntax, FieldDecl, Name) \
  E(Syntax, FieldDecl, SemiColon) \
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
  /* Modifier */ \
  E(Syntax, Modifier, Duplicate) \
  /* NamespaceDecl */ \
  E(Syntax, NamespaceDecl, LeftCurryBracket) \
  E(Syntax, NamespaceDecl, NameDuplicate) \
  E(Syntax, NamespaceDecl, RightCurryBracket) \
  /* UsingDirective */ \
  E(Syntax, UsingDirective, Invalid) \
  E(Syntax, UsingDirective, Name) \
  E(Syntax, UsingDirective, SemiColon)

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

#endif // !defined(INCLUDE_elang_compiler_public_compiler_error_code_h)

