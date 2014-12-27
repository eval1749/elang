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

std::ostream& operator<<(std::ostream& ostream, const Modifiers& modifiers) {
  static const char* const strings[] = {
    #define STRING(name, string, details) string,
    MODIFIER_LIST(STRING)
    #undef STRING
  };
  const char* separator = "";
  const char* const* name = strings;
  auto value = modifiers.value();
  while (value) {
    if (value & 1) {
      ostream << separator << *name;
      separator = " ";
    }
    value >>= 1;
    ++name;
  }
  return ostream;
}

}  // namespace compiler
}  // namespace elang
