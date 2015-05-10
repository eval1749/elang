// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_PREDEFINED_NAMES_H_
#define ELANG_COMPILER_PREDEFINED_NAMES_H_

#include <array>
#include <iosfwd>

#include "base/macros.h"

namespace elang {
class AtomicString;
namespace compiler {
class TokenFactory;

//////////////////////////////////////////////////////////////////////
//
// PredefinedName holds list of predefined type names in `System` namespace.
//
#define FOR_EACH_PREDEFINED_NAME(V) \
  V(Bool)                           \
  V(Char)                           \
  V(Float32)                        \
  V(Float64)                        \
  V(Int16)                          \
  V(Int32)                          \
  V(Int64)                          \
  V(Int8)                           \
  V(IntPtr)                         \
  V(Object)                         \
  V(String)                         \
  V(UInt16)                         \
  V(UInt32)                         \
  V(UInt64)                         \
  V(UInt8)                          \
  V(UIntPtr)                        \
  V(ValueType)                      \
  V(Void)

enum class PredefinedName {
#define V(Name) Name,
  FOR_EACH_PREDEFINED_NAME(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, PredefinedName name);

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_PREDEFINED_NAMES_H_
