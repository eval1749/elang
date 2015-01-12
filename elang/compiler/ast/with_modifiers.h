// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_WITH_MODIFIERS_H_
#define ELANG_COMPILER_AST_WITH_MODIFIERS_H_

#include "elang/compiler/modifiers.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// WithModifiers
//
class WithModifiers {
 public:
  Modifiers modifiers() const { return modifiers_; }

 protected:
  explicit WithModifiers(Modifiers modifiers);
  ~WithModifiers();

 private:
  Modifiers modifiers_;

  DISALLOW_COPY_AND_ASSIGN(WithModifiers);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_WITH_MODIFIERS_H_