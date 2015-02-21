// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_STACK_ASSIGNER_H_
#define ELANG_LIR_TRANSFORMS_STACK_ASSIGNER_H_

#include "base/macros.h"
#include "elang/lir/factory_user.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/transforms/register_assignments.h"

namespace elang {
namespace lir {

class Instruction;
class RegisterAssignments;
class StackAssignments;

//////////////////////////////////////////////////////////////////////
//
// StackAssigner
//
class ELANG_LIR_EXPORT StackAssigner final : public FactoryUser {
 public:
  explicit StackAssigner(Factory* factory,
                         RegisterAssignments* register_assignments,
                         StackAssignments* stack_assignments);
  ~StackAssigner();

  void Run();

 private:
  void AddEpilogue(Instruction* instruction);
  void AddPrologue(Instruction* instruction);
  void RunForLeafFunction();
  void RunForNonLeafFunction();
  void SetStackSlot(Value proxy, Value stack_slot);

  RegisterAssignments::Editor register_assignments_;
  StackAssignments* const stack_assignments_;

  DISALLOW_COPY_AND_ASSIGN(StackAssigner);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_STACK_ASSIGNER_H_
