// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/compiler/predefined_names.h"

#include "elang/compiler/token_factory.h"

namespace elang {
namespace compiler {

PredefinedNames::PredefinedNames(TokenFactory* factory) {
#define V(Name)                                       \
  names_[static_cast<size_t>(PredefinedName::Name)] = \
      factory->NewAtomicString(L## #Name);
  FOR_EACH_PREDEFINED_NAME(V)
#undef V
}

PredefinedNames::~PredefinedNames() {
}

AtomicString* PredefinedNames::AsAtomicString(PredefinedName name) const {
  return names_[static_cast<size_t>(name)];
}

std::ostream& operator<<(std::ostream& ostream, PredefinedName name) {
  static const char* const names[] = {
#define V(Name) "System." #Name,
      FOR_EACH_PREDEFINED_NAME(V)
#undef V
          "Invalid",
  };
  return ostream
         << names[std::min(static_cast<size_t>(name), arraysize(names) - 1)];
}

}  // namespace compiler
}  // namespace elang
