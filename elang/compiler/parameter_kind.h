// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_PARAMETER_KIND_H_
#define ELANG_COMPILER_PARAMETER_KIND_H_

#include <ostream>

namespace elang {
namespace compiler {

#define FOR_EACH_PARAMETER_KIND(V) \
  V(Required)                      \
  V(Optional)                      \
  V(Rest)

enum class ParameterKind {
#define V(Name) Name,
  FOR_EACH_PARAMETER_KIND(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, ParameterKind kind);

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_PARAMETER_KIND_H_
