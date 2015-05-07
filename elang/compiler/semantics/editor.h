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
  Editor(CompilationSession* session, Factory* factory);
  ~Editor();

  Factory* factory() const { return factory_; }

  void SetSemanticOf(ast::Node* node, sm::Semantic* semantic);

 private:
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace sm
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SEMANTICS_EDITOR_H_
