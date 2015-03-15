// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_MACHINE_CODE_ANNOTATION_H_
#define ELANG_VM_MACHINE_CODE_ANNOTATION_H_

#include "base/basictypes.h"

namespace elang {
namespace vm {

#define FOR_EACH_CODE_ANNOTATION_KIND(V) \
  V(Invalid)                             \
  V(Block)                               \
  V(CallSite)                            \
  V(Function)                            \
  V(Float32)                             \
  V(Float64)                             \
  V(Int32)                               \
  V(Int64)                               \
  V(Object)                              \
  V(UInt32)                              \
  V(UInt64)

//////////////////////////////////////////////////////////////////////
//
// MachineCodeAnnotation
//
struct MachineCodeAnnotation final {
  enum Kind : uint32_t {
#define V(Name) Name,
    FOR_EACH_CODE_ANNOTATION_KIND(V)
#undef V
  };

  Kind kind : 4;
  uint32_t offset : 28;
};

static_assert(sizeof(MachineCodeAnnotation) == sizeof(int),
              "MachineCodeAnnotation must be compatible int");

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_MACHINE_CODE_ANNOTATION_H_
