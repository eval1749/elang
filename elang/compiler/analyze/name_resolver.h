// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_
#define ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_

#include <unordered_map>

#include "base/macros.h"

namespace elang {
namespace compiler {
namespace ast {
class Expression;
class NamespaceMember;
}

class CompilationSession;

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

  ast::NamespaceMember* FindReference(ast::Expression* reference);
  void Resolved(ast::Expression* reference, ast::NamespaceMember* member);

 private:
  std::unordered_map<ast::Expression*, ast::NamespaceMember*> map_;
  CompilationSession* session_;

  DISALLOW_COPY_AND_ASSIGN(NameResolver);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_NAME_RESOLVER_H_
