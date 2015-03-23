// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/nodes.h"
#include "elang/optimizer/thing.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

std::ostream& operator<<(std::ostream& ostream, const Thing& thing) {
  if (auto const node = thing.as<const Node>())
    return ostream << *node;
  if (auto const type = thing.as<const Type>())
    return ostream << *type;
  return ostream << &thing;
}

}  // namespace optimizer
}  // namespace elang
