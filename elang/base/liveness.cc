// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/liveness.h"

namespace elang {

Liveness::Liveness(Zone* zone, int size)
    : in_(zone, size), kill_(zone, size), out_(zone, size) {
}

std::ostream& operator<<(std::ostream& ostream, const Liveness& liveness) {
  ostream << "{in:" << liveness.in() << ", out:" << liveness.out()
          << ", kill:" << liveness.kill() << "}";
  return ostream;
}

}  // namespace elang
