// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SEMANTICS_EDITOR_H_
#define ELANG_COMPILER_SEMANTICS_EDITOR_H_

#include "base/macros.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {
class CompilationSession;
namespace ast {
class Node;
}
namespace sm {
class Factory;
class Semantic;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class Editor final : public CompilationSessionUser {
 public:
  explicit Editor(CompilationSession* session);
  ~Editor();

  Factory* factory() const;

  void SetSemanticOf(ast::Node* node, Semantic* semantic);
  Semantic* SemanticOf(ast::Node* node) const;
  Semantic* TrySemanticOf(ast::Node* node) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace sm
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SEMANTICS_EDITOR_H_
