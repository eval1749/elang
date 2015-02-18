// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/register_allocation_pass.h"

#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/transforms/register_allocator.h"
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/transforms/register_usage_tracker.h"
#include "elang/lir/transforms/stack_allocator.h"
#include "elang/lir/transforms/stack_assigner.h"
#include "elang/lir/transforms/stack_assignments.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// RegisterAssignmentsPass
//
RegisterAssignmentsPass::RegisterAssignmentsPass(Editor* editor)
    : FunctionPass(editor) {
}

RegisterAssignmentsPass::~RegisterAssignmentsPass() {
}

base::StringPiece RegisterAssignmentsPass::name() const {
  return "register_allocation";
}

void RegisterAssignmentsPass::RunOnFunction() {
  RegisterAssignments register_assignments;
  RegisterUsageTracker usage_tracker(editor());
  StackAssignments stack_assignments;
  StackAllocator stack_allocator(&stack_assignments,
                                 Target::PointerSizeInByte());
  RegisterAllocator allocator(editor(), &register_assignments,
                              editor()->AnalyzeLiveness(), usage_tracker,
                              &stack_allocator);
  allocator.Run();

  {
    StackAssigner stack_assigner(factory(), &register_assignments,
                                 &stack_assignments);
    stack_assigner.Run();
  }

  for (auto const block : function()->basic_blocks()) {
    editor()->Edit(block);
    for (auto const instr : block->instructions()) {
      for (auto const action : register_assignments.BeforeActionOf(instr))
        ProcessInstruction(register_assignments, action);
      ProcessInstruction(register_assignments, instr);
    }
    editor()->Commit();
  }
}

void RegisterAssignmentsPass::ProcessInstruction(
    const RegisterAssignments& register_assignments,
    Instruction* instr) {
  auto output_position = 0;
  for (auto const output : instr->outputs()) {
    auto const allocation = register_assignments.AllocationOf(instr, output);
    editor()->SetOutput(instr, output_position, allocation);
    ++output_position;
  }
  auto input_position = 0;
  for (auto const input : instr->inputs()) {
    auto const allocation = register_assignments.AllocationOf(instr, input);
    editor()->SetInput(instr, input_position, allocation);
    ++input_position;
  }
}

}  // namespace lir
}  // namespace elang
