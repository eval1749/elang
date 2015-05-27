// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_WITH_MODIFIERS_H_
#define ELANG_COMPILER_WITH_MODIFIERS_H_

#include "elang/compiler/modifiers.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// WithModifiers
//
class WithModifiers {
 public:
  Modifiers modifiers() const { return modifiers_; }

#define V(name, string, details) \
  bool Is##name() const { return modifiers_.Has##name(); }
  FOR_EACH_MODIFIER(V)
#undef V

 protected:
  explicit WithModifiers(Modifiers modifiers);
  ~WithModifiers();

 private:
  Modifiers modifiers_;

  DISALLOW_COPY_AND_ASSIGN(WithModifiers);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_WITH_MODIFIERS_H_
