// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_MODIFIERS_BUILDER_H_
#define ELANG_COMPILER_MODIFIERS_BUILDER_H_

#include <string>

#include "elang/compiler/modifiers.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// ModifiersBuilder
//
class ModifiersBuilder final {
 public:
  ModifiersBuilder();
  ~ModifiersBuilder();

  Modifiers Get() const;
  void Reset();

#define DECL_ACCESSOR(name, string, details) \
  bool Has##name() const;                    \
  void Set##name();
  FOR_EACH_MODIFIER(DECL_ACCESSOR)
#undef DECL_ACCESSOR

 private:
  int flags_;

  DISALLOW_COPY_AND_ASSIGN(ModifiersBuilder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_MODIFIERS_BUILDER_H_
