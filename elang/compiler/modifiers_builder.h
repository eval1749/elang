// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_modifiers_builder_h)
#define INCLUDE_elang_compiler_modifiers_builder_h

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
    bool Has ## name() const; \
    void Set ## name();
  MODIFIER_LIST(DECL_ACCESSOR)
  #undef DECL_ACCESSOR

 private:
  int flags_;

  DISALLOW_COPY_AND_ASSIGN(ModifiersBuilder);
};

}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_modifiers_builder_h)

