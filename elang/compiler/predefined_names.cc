// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "elang/compiler/predefined_names.h"

namespace elang {
namespace compiler {

std::ostream& operator<<(std::ostream& ostream, PredefinedName name) {
  static const char* const names[] = {
#define V(Name) "System." #Name,
      FOR_EACH_PREDEFINED_NAME(V)
#undef V
  };
  auto const it = std::begin(names) + static_cast<size_t>(name);
  if (it < std::begin(names) || it >= std::end(names))
    return ostream << "Invalid";
  return ostream << *it;
}

}  // namespace compiler
}  // namespace elang
