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
  RegisterAssignments allocations;
  RegisterUsageTracker usage_tracker(editor());
  StackAssigments stack_assignments;
  StackAllocator stack_allocator(&stack_assignments,
                                 Target::PointerSizeInByte());
  RegisterAllocator allocator(editor(), &allocations,
                              editor()->AnalyzeLiveness(), usage_tracker,
                              &stack_allocator);
  allocator.Run();

  for (auto const block : function()->basic_blocks()) {
    editor()->Edit(block);
    for (auto const instr : block->instructions()) {
      for (auto const action : allocations.BeforeActionOf(instr))
        ProcessInstruction(allocations, action);
      ProcessInstruction(allocations, instr);
    }
    editor()->Commit();
  }
}

void RegisterAssignmentsPass::ProcessInstruction(
    const RegisterAssignments& allocations,
    Instruction* instr) {
  auto output_position = 0;
  for (auto const output : instr->outputs()) {
    auto const allocation = allocations.AllocationOf(instr, output);
    editor()->SetOutput(instr, output_position, allocation);
    ++output_position;
  }
  auto input_position = 0;
  for (auto const input : instr->inputs()) {
    auto const allocation = allocations.AllocationOf(instr, input);
    editor()->SetInput(instr, input_position, allocation);
    ++input_position;
  }
}

}  // namespace lir
}  // namespace elang
