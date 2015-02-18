// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_STACK_ASSIGNER_H_
#define ELANG_LIR_TRANSFORMS_STACK_ASSIGNER_H_

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// StackAssigner
//
class ELANG_LIR_EXPORT StackAssigner final {
 public:
  explicit StackAssigner(RegisterAssignments* assignments,
                         const StackAssignments& stack_assignments);
  ~StackAssigner();

  void Run();

 private:
  void RunForLeafFunction();
  void RunForNonLeafFunction();

  const StackAssignments* const assignments_;
  RegisterAssignments* const register_assignments_;

  DISALLOW_COPY_AND_ASSIGN(StackAssigner);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_STACK_ASSIGNER_H_
