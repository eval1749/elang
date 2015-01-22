// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INTRINSIC_NAMES_H_
#define ELANG_HIR_INTRINSIC_NAMES_H_

#include <ostream>

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Intrinsic function names
//
#define FOR_EACH_INTRINSIC_NAME(V) \
  V(HeapAlloc)                     \
  V(StackAlloc)

enum class IntrinsicName {
#define V(Name) Name,
  FOR_EACH_INTRINSIC_NAME(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, IntrinsicName name);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INTRINSIC_NAMES_H_
