// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/semantics/editor.h"

#include "base/logging.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/semantics.h"

namespace elang {
namespace compiler {
namespace sm {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(CompilationSession* session)
    : CompilationSessionUser(session) {
}

Editor::~Editor() {
}

sm::Factory* Editor::factory() const {
  return session()->semantics_factory();
}

void Editor::SetSemanticOf(ast::Node* node, sm::Semantic* semantic) {
  DCHECK(node);
  DCHECK(semantic);
  semantics()->semantic_map_[node] = semantic;
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
