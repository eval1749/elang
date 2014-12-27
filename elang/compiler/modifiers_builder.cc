// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/modifiers_builder.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// ModifiersBuilder
//
ModifiersBuilder::ModifiersBuilder() : flags_(0) {
}

ModifiersBuilder::~ModifiersBuilder() {
}

Modifiers ModifiersBuilder::Get() const {
  return Modifiers(flags_);
}

void ModifiersBuilder::Reset() {
  flags_ = 0;
}

#define DEFINE_ACCESSOR(name, details) \
  bool ModifiersBuilder::Has ## name() const { \
    return (flags_ & (1 << static_cast<int>(Modifier::name))) != 0; \
  } \
  void ModifiersBuilder::Set ## name() { \
    flags_ |= 1 << static_cast<int>(Modifier::name);\
  }
MODIFIER_LIST(DEFINE_ACCESSOR)
#undef DEFINE_ACCESSOR


}  // namespace compiler
}  // namespace elang
