// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_
#define ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "base/macros.h"
#include "base/strings/string_piece.h"

namespace elang {
namespace compiler {
namespace ast {
class Class;
class Expression;
class MemberContainer;
class Namespace;
class NamespaceMember;
}

class CompilationSession;
class Token;
class TokenData;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// NameResolver
// Keeps analysis results from |NamespaceAnalyzer| for mapping name reference
// to |ast::NamespaceMember|.
//
class NameResolver final {
 public:
  explicit NameResolver(CompilationSession* session);
  ~NameResolver();

  void DidResolveReference(ast::Expression* reference,
                           ast::NamespaceMember* member);
  ast::NamespaceMember* FindReference(ast::Expression* reference);

 private:
  std::unordered_map<ast::Expression*, ast::NamespaceMember*> map_;
  std::unordered_map<TokenType, ast::Class*> keyword_types_;
  CompilationSession* const session_;

  DISALLOW_COPY_AND_ASSIGN(NameResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_
