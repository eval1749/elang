// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/hir/intrinsic_names.h"

#include "base/macros.h"

namespace elang {
namespace hir {

std::ostream& operator<<(std::ostream& ostream, IntrinsicName name) {
  static const char* const names[] = {
#define V(Name) "System.Intrinsic." #Name,
      FOR_EACH_INTRINSIC_NAME(V)
#undef V
          "Invalid",
  };
  return ostream
         << names[std::min(static_cast<size_t>(name), arraysize(names) - 1)];
}

}  // namespace hir
}  // namespace elang
