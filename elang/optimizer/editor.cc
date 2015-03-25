// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/editor.h"

#include "base/logging.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(Factory* factory, Function* function)
    : ErrorReporter(factory), FactoryUser(factory), function_(function) {
}

Editor::~Editor() {
}

void Editor::SetInput(Node* node, size_t index, Node* new_value) {
  DCHECK(new_value->IsValidData()) << *new_value;
  DCHECK_NE(node, new_value);
  DCHECK_LE(new_value->id(), function_->max_node_id());
  node->InputAt(index)->SetValue(new_value);
}

}  // namespace optimizer
}  // namespace elang
