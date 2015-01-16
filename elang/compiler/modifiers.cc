// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

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

bool Modifiers::operator==(const Modifiers& other) const {
  return flags_ == other.flags_;
}

bool Modifiers::operator!=(const Modifiers& other) const {
  return flags_ != other.flags_;
}

Modifiers Modifiers::operator&(const Modifiers& other) const {
  return Modifiers(flags_ & other.flags_);
}

Modifiers Modifiers::operator|(const Modifiers& other) const {
  return Modifiers(flags_ | other.flags_);
}

Modifiers Modifiers::operator^(const Modifiers& other) const {
  return Modifiers(flags_ ^ other.flags_);
}

Modifiers Modifiers::Class() {
  return Modifiers(Modifier::Abstract, Modifier::Final, Modifier::New,
                   Modifier::Partial, Modifier::Private, Modifier::Protected,
                   Modifier::Public, Modifier::Static);
}

Modifiers Modifiers::Enum() {
  return Modifiers(Modifier::New, Modifier::Private, Modifier::Protected,
                   Modifier::Public);
}

Modifiers Modifiers::Field() {
  return Modifiers(Modifier::Abstract, Modifier::Final, Modifier::New,
                   Modifier::Private, Modifier::Protected, Modifier::Public,
                   Modifier::Static, Modifier::Volatile);
}

Modifiers Modifiers::Method() {
  return Modifiers(Modifier::Abstract, Modifier::Extern, Modifier::Final,
                   Modifier::New, Modifier::Override, Modifier::Partial,
                   Modifier::Private, Modifier::Protected, Modifier::Public,
                   Modifier::Static, Modifier::Virtual);
}

std::ostream& operator<<(std::ostream& ostream, const Modifiers& modifiers) {
  static const char* const strings[] = {
#define STRING(name, string, details) string,
      FOR_EACH_MODIFIER(STRING)
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
