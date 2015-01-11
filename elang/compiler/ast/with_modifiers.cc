// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/with_modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// WithModifiers
//
WithModifiers::WithModifiers(Modifiers modifiers) : modifiers_(modifiers) {
}

WithModifiers::~WithModifiers() {
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
