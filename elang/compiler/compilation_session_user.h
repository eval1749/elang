// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_COMPILATION_SESSION_USER_H_
#define ELANG_COMPILER_COMPILATION_SESSION_USER_H_

#include "base/macros.h"

namespace elang {
namespace compiler {

namespace ast {
class Class;
class Namespace;
class NamespaceBody;
class Node;
}
namespace sm {
class Type;
}
class Analysis;
class CompilationSession;
enum class ErrorCode;
enum class PredefinedName;
class Token;

//////////////////////////////////////////////////////////////////////
//
// CompilationSessionUser
//
class CompilationSessionUser {
 public:
  ~CompilationSessionUser();

  CompilationSession* session() const { return session_; }

  // Report error caused by |node|.
  void Error(ErrorCode error_code, ast::Node* node);
  void Error(ErrorCode error_code, Token* node);
  void Error(ErrorCode error_code, ast::Node* node, ast::Node* node2);
  void Error(ErrorCode error_code, Token* node, Token* node2);

 protected:
  explicit CompilationSessionUser(CompilationSession* session);

  Analysis* analysis() const;
  ast::Namespace* system_namespace();
  ast::NamespaceBody* system_namespace_body();

  sm::Type* PredefinedTypeOf(PredefinedName name);
  Token* PrettyTokenFor(ast::Node* node);

 private:
  CompilationSession* const session_;

  DISALLOW_COPY_AND_ASSIGN(CompilationSessionUser);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_COMPILATION_SESSION_USER_H_
