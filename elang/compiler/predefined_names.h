// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_PREDEFINED_NAMES_H_
#define ELANG_COMPILER_PREDEFINED_NAMES_H_

#include <array>
#include <ostream>

#include "base/macros.h"
#include "elang/compiler/ast/nodes_forward.h"

namespace elang {
class AtomicString;
namespace compiler {
class CompilationSession;

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
  V(Object)                         \
  V(String)                         \
  V(UInt16)                         \
  V(UInt32)                         \
  V(UInt64)                         \
  V(UInt8)                          \
  V(ValueType)                      \
  V(Void)

enum class PredefinedName {
#define V(Name) Name,
  FOR_EACH_PREDEFINED_NAME(V)
#undef V
};

// |kNumberOfPredefinedNames| must be synced with |FOR_EACH_PREDEFINED_NAME|.
const size_t kNumberOfPredefinedNames =
    static_cast<size_t>(PredefinedName::Void) + 1;

//////////////////////////////////////////////////////////////////////
//
// PredefinedNames
//
class PredefinedNames final {
 public:
  explicit PredefinedNames(CompilationSession* session);
  ~PredefinedNames();

  AtomicString* name_for(PredefinedName name) const;

 private:
  std::array<AtomicString*, kNumberOfPredefinedNames> names_;

  DISALLOW_COPY_AND_ASSIGN(PredefinedNames);
};

std::ostream& operator<<(std::ostream& ostream, PredefinedName name);

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_PREDEFINED_NAMES_H_
