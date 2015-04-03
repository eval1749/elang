// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_OPCODE_H_
#define ELANG_OPTIMIZER_OPCODE_H_

#include "elang/optimizer/nodes_forward.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Opcode
//
enum class Opcode {
#define V(Name, ...) Name,
  FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V
      NumberOfOpcodes,
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_OPCODE_H_
