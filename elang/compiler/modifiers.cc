// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/modifiers.h"

#include "base/logging.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Modifiers
//
Modifiers::Modifiers(const Modifiers& other) : flags_(other.flags_) {
}

Modifiers::Modifiers(int flags) : flags_(flags) {
}

Modifiers::Modifiers() : Modifiers(0) {
}

Modifiers::~Modifiers() {
}

Modifiers& Modifiers::operator=(const Modifiers& other) {
  flags_ = other.flags_;
  return *this;
}

}  // namespace compiler
}  // namespace elang
