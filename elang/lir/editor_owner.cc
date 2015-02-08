// Copyright 2015 Project Vogue. All rights reserved.
// Owner of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/editor_owner.h"

#include "elang/lir/editor.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// EditorOwner
//
EditorOwner::EditorOwner(Factory* factory, Function* function)
    : EditorUser(new Editor(factory, function)), editor_(editor()) {
}

EditorOwner::~EditorOwner() {
}

}  // namespace lir
}  // namespace elang
