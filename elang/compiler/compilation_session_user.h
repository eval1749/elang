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
}

class CompilationSession;
enum class PredefinedName;
class Semantics;

//////////////////////////////////////////////////////////////////////
//
// CompilationSessionUser
//
class CompilationSessionUser {
 public:
  ~CompilationSessionUser();

  CompilationSession* session() const { return session_; }

 protected:
  explicit CompilationSessionUser(CompilationSession* session);

  Semantics* semantics() const;
  ast::Namespace* system_namespace();
  ast::NamespaceBody* system_namespace_body();

  ast::Class* GetPredefinedType(PredefinedName name);

 private:
  CompilationSession* const session_;

  DISALLOW_COPY_AND_ASSIGN(CompilationSessionUser);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_COMPILATION_SESSION_USER_H_
