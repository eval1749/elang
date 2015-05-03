// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/transforms/register_allocation_pass.h"

#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/transforms/register_allocator.h"
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/transforms/stack_allocator.h"
#include "elang/lir/transforms/stack_assigner.h"
#include "elang/lir/transforms/stack_assignments.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

namespace {
bool IsUselessInstruction(const Instruction* instr) {
  if (instr->is<CopyInstruction>())
    return instr->output(0) == instr->input(0);
  return false;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RegisterAssignmentsPass
//
RegisterAssignmentsPass::RegisterAssignmentsPass(Editor* editor)
    : FunctionPass(editor),
      register_assignments_(new RegisterAssignments()),
      stack_assignments_(new StackAssignments()) {
}

RegisterAssignmentsPass::~RegisterAssignmentsPass() {
}

base::StringPiece RegisterAssignmentsPass::name() const {
  return "register_allocation";
}

Value RegisterAssignmentsPass::AssignmentOf(Instruction* instr,
                                            Value operand) const {
  if (!operand.is_virtual())
    return operand;
  auto const assignment = register_assignments_->AllocationOf(instr, operand);
  if (assignment.is_physical())
    return assignment;
  if (assignment.is_spill_slot())
    return stack_assignments_->StackSlotOf(operand);
  NOTREACHED() << "unexpected assignment for " << operand << " " << assignment;
  return Value();
}

void RegisterAssignmentsPass::RunOnFunction() {
  {
    RegisterAllocator allocator(editor(), register_assignments_.get(),
                                stack_assignments_.get());
    allocator.Run();
  }

  {
    StackAssigner stack_assigner(factory(), register_assignments_.get(),
                                 stack_assignments_.get());
    stack_assigner.Run();
  }

  // Insert prologue
  {
    auto const entry_block = editor()->entry_block();
    auto const entry_instr = entry_block->first_instruction();
    auto const ref_instr = entry_instr->next();
    editor()->Edit(entry_block);
    for (auto const instr : stack_assignments_->prologue())
      editor()->InsertBefore(instr, ref_instr);
    editor()->Commit();
  }

  for (auto const block : function()->basic_blocks()) {
    Editor::ScopedEdit scope(editor());
    editor()->Edit(block);
    WorkList<Instruction> action_owners;
    for (auto const instr : block->instructions()) {
      if (!register_assignments_->BeforeActionOf(instr).empty())
        action_owners.Push(instr);
      ProcessInstruction(instr);
    }
    while (!action_owners.empty()) {
      auto const instr = action_owners.Pop();
      for (auto const action : register_assignments_->BeforeActionOf(instr)) {
        editor()->InsertBefore(action, instr);
        ProcessInstruction(action);
      }
      if (!instr->is<PCopyInstruction>())
        continue;
      editor()->Remove(instr);
    }
    auto const ret_instr = block->last_instruction()->as<RetInstruction>();
    if (!ret_instr)
      continue;
    // Insert epilogue before 'ret' instruction.
    for (auto const instr : stack_assignments_->epilogue())
      editor()->InsertBefore(instr, ret_instr);
  }

  editor()->BulkRemoveInstructions(&useless_instructions_);
}

void RegisterAssignmentsPass::ProcessInstruction(Instruction* instr) {
  auto output_position = 0;
  for (auto const output : instr->outputs()) {
    auto const assignment = AssignmentOf(instr, output);
    editor()->SetOutput(instr, output_position, assignment);
    ++output_position;
  }
  auto input_position = 0;
  for (auto const input : instr->inputs()) {
    auto const assignment = AssignmentOf(instr, input);
    editor()->SetInput(instr, input_position, assignment);
    ++input_position;
  }
  if (!IsUselessInstruction(instr))
    return;
  useless_instructions_.Push(instr);
}

}  // namespace lir
}  // namespace elang
