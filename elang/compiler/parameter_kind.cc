// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/compiler/parameter_kind.h"

#include "base/macros.h"

namespace elang {
namespace compiler {

std::ostream& operator<<(std::ostream& ostream, ParameterKind kind) {
  static const char* const texts[] = {
#define V(Name) #Name,
      FOR_EACH_PARAMETER_KIND(V)
#undef V
          "Invalid",
  };
  return ostream
         << texts[std::min(static_cast<size_t>(kind), arraysize(texts) - 1)];
}

}  // namespace compiler
}  // namespace elang
