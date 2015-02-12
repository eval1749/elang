// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/editor_user.h"

#include "elang/lir/editor.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// EditorUser
//
EditorUser::EditorUser(Editor* editor) : editor_(editor) {
}

EditorUser::~EditorUser() {
}

Function* EditorUser::function() const {
  return editor_->function();
}

}  // namespace lir
}  // namespace elang
