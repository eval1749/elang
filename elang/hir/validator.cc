// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "elang/hir/validator.h"

#include "base/macros.h"
#include "elang/hir/analysis/dominator_tree.h"
#include "elang/hir/editor.h"
#include "elang/hir/error_code.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

namespace {

Type* PointTo(Type* type) {
  auto const pointer_type = type->as<PointerType>();
  return pointer_type ? pointer_type->pointee() : nullptr;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Validator
//
Validator::Validator(Editor* editor)
    : TypeFactoryUser(editor->types()),
      dominator_tree_(editor->maybe_dominator_tree()),
      editor_(editor),
      is_valid_(false) {
}

Validator::~Validator() {
}

// Returns true if |dominator| dominates |dominatee|.
bool Validator::Dominates(Value* dominator, Instruction* dominatee) {
  DCHECK(dominator_tree_);
  auto const dominator_instruction = dominator->as<Instruction>();
  if (!dominator_instruction) {
    // Literal always available.
    return true;
  }
  if (!dominator_instruction->basic_block())
    return false;
  return dominator_tree_->Dominates(dominator_instruction->basic_block(),
                                    dominatee->basic_block());
}

void Validator::Error(ErrorCode error_code, const Value* error_value) {
  is_valid_ = false;
  editor()->Error(error_code, error_value);
}

void Validator::Error(ErrorCode error_code, const Value* value, Thing* detail) {
  is_valid_ = false;
  editor()->Error(error_code, value, detail);
}

void Validator::Error(ErrorCode error_code,
                      const Value* error_value,
                      const std::vector<Thing*>& details) {
  is_valid_ = false;
  editor()->Error(error_code, error_value, details);
}

void Validator::Error(ErrorCode error_code,
                      const Instruction* instruction,
                      int index) {
  is_valid_ = false;
  editor()->Error(error_code, instruction, {NewInt32(index)});
}

void Validator::Error(ErrorCode error_code,
                      const Instruction* instruction,
                      int index,
                      Thing* detail) {
  is_valid_ = false;
  editor()->Error(error_code, instruction, {NewInt32(index), detail});
}

Value* Validator::NewInt32(int32_t data) {
  return editor()->NewInt32(data);
}

// Validates |BasicBlock|
//  - id() in list
//  - function() in list
//  - terminator at the last
//  - instructions
bool Validator::Validate(BasicBlock* block) {
  if (!block->id()) {
    Error(ErrorCode::ValidateBasicBlockId, block);
    return false;
  }
  if (!block->function()) {
    Error(ErrorCode::ValidateBasicBlockFunction, block);
    return false;
  }
  if (block->instructions().empty()) {
    Error(ErrorCode::ValidateBasicBlockEmpty, block);
    return false;
  }

  // Entry block
  auto const entry_block = block->function()->entry_block();
  if (block == entry_block) {
    if (!block->first_instruction()->is<EntryInstruction>()) {
      Error(ErrorCode::ValidateInstructionEntry, block);
      return false;
    }
    if (entry_block->HasPredecessor()) {
      Error(ErrorCode::ValidateBasicBlockEntry, block);
      return false;
    }
  } else if (block->first_instruction()->is<EntryInstruction>()) {
    Error(ErrorCode::ValidateInstructionEntry, block->first_instruction());
    return false;
  }

  // Exit block
  auto const exit_block = block->function()->exit_block();
  if (block == exit_block) {
    if (!block->first_instruction()->is<ExitInstruction>()) {
      Error(ErrorCode::ValidateInstructionExit, block->first_instruction());
      return false;
    }
    if (exit_block->HasSuccessor()) {
      Error(ErrorCode::ValidateBasicBlockExit, exit_block);
      return false;
    }
  } else if (block->last_instruction()->is<ExitInstruction>()) {
    Error(ErrorCode::ValidateInstructionExit, block->last_instruction());
    return false;
  }

  // Check instructions
  auto found_terminator = false;
  auto is_valid = true;
  for (auto instruction : block->instructions()) {
    if (!instruction->id()) {
      Error(ErrorCode::ValidateInstructionId, instruction);
      return false;
    }
    if (instruction->IsTerminator()) {
      if (found_terminator) {
        Error(ErrorCode::ValidateInstructionTerminator, instruction);
        return false;
      }
      found_terminator = true;
    }
    if (!Validate(instruction))
      is_valid = false;
  }
  if (!found_terminator) {
    Error(ErrorCode::ValidateBasicBlockTerminator, block);
    return false;
  }
  return is_valid;
}

bool Validator::Validate(Function* function) {
  if (function->basic_blocks().empty()) {
    Error(ErrorCode::ValidateFunctionEmpty, function);
    return false;
  }
  auto const entry_block = function->entry_block();
  if (!entry_block->first_instruction()->is<EntryInstruction>()) {
    Error(ErrorCode::ValidateFunctionEntry, function);
    return false;
  }

  auto const exit_block = function->exit_block();
  if (!exit_block->first_instruction()->is<ExitInstruction>()) {
    Error(ErrorCode::ValidateFunctionExit, function);
    return false;
  }

  auto is_valid = true;
  for (auto block : function->basic_blocks()) {
    if (!Validate(block))
      is_valid = false;
  }
  return is_valid;
}

bool Validator::Validate(Instruction* instruction) {
  if (!instruction->id()) {
    Error(ErrorCode::ValidateInstructionId, instruction);
    return false;
  }
  if (!instruction->basic_block()) {
    Error(ErrorCode::ValidateInstructionBasicBlock, instruction);
    return false;
  }
  is_valid_ = true;
  // Check instruction inputs are alive.
  // Note: Event if inputs are alive, they are wrong when users of inputs
  // aren't dominated where inputs are set.
  // Example:
  //  block1:
  //    use %r1
  //    br block2
  //  block2:
  //    def %r1
  //    ...
  //  where block1 isn't dominated by block2.
  auto position = 0;
  for (auto input : instruction->inputs()) {
    if (auto const instr = input->as<Instruction>()) {
      if (!instr->id() || !instr->basic_block()->id())
        Error(ErrorCode::ValidateInstructionOrphan, instr, position);
      else if (dominator_tree_ && !Dominates(input, instruction))
        Error(ErrorCode::ValidateInstructionDominance, instr, position);
    }
    ++position;
  }
  instruction->Accept(this);
  return is_valid_;
}

void Validator::ValidateArrayAccess(Instruction* instr) {
  auto const array_pointer = instr->input(0);
  auto const array_type = PointTo(array_pointer->type())->as<ArrayType>();
  if (!array_type) {
    Error(ErrorCode::ValidateInstructionType, instr, 0);
    return;
  }
  auto const indexes = instr->input(1);
  if (array_type->rank() == 1) {
    if (indexes->type() != int32_type())
      Error(ErrorCode::ValidateInstructionType, instr, 1);
    return;
  }
  auto const indexes_type = indexes->type()->as<TupleType>();
  if (!indexes_type) {
    Error(ErrorCode::ValidateInstructionType, instr, 1);
    return;
  }
  if (array_type->rank() != indexes_type->size()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1);
    return;
  }
  for (auto const member : indexes_type->members()) {
    if (member != int32_type())
      Error(ErrorCode::ValidateInstructionType, instr, 1);
  }
}

// InstructionVisitor
#define V(Name, ...)                                       \
  void Validator::Visit##Name(Name##Instruction* instr) {  \
    if (instr->input(0)->type() != instr->output_type())   \
      Error(ErrorCode::ValidateInstructionType, instr, 0); \
    if (instr->input(1)->type() != instr->output_type())   \
      Error(ErrorCode::ValidateInstructionType, instr, 1); \
  }
FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
FOR_EACH_BITWISE_BINARY_OPERATION(V)
#undef V

#define V(Name, ...)                                       \
  void Validator::Visit##Name(Name##Instruction* instr) {  \
    if (instr->input(0)->type() != instr->output_type())   \
      Error(ErrorCode::ValidateInstructionType, instr, 0); \
    if (instr->input(1)->type() != int32_type())           \
      Error(ErrorCode::ValidateInstructionType, instr, 1); \
  }
FOR_EACH_BITWISE_SHIFT_OPERATION(V)
#undef V

#define V(Name, ...)                                        \
  void Validator::Visit##Name(Name##Instruction* instr) {   \
    if (instr->input(1)->type() != instr->input(0)->type()) \
      Error(ErrorCode::ValidateInstructionType, instr, 1);  \
  }
FOR_EACH_EQUALITY_OPERATION(V)
#undef V

#define V(Name, ...)                                        \
  void Validator::Visit##Name(Name##Instruction* instr) {   \
    if (!instr->input(0)->type()->is_numeric())             \
      Error(ErrorCode::ValidateInstructionType, instr, 0);  \
    if (instr->input(1)->type() != instr->input(0)->type()) \
      Error(ErrorCode::ValidateInstructionType, instr, 1);  \
  }
FOR_EACH_RELATIONAL_OPERATION(V)
#undef V

#define V(Name, ...) \
  void Validator::Visit##Name(Name##Instruction* instr) { DCHECK(instr); }
FOR_EACH_TYPE_CAST_OPERATION(V)
#undef V

void Validator::VisitBound(BoundInstruction* instr) {
  ValidateArrayAccess(instr);
}

void Validator::VisitBranch(BranchInstruction* instr) {
  // Since, we use use-def list for representing predecessors of basic block,
  // basic block must be unique in inputs.
  DCHECK_NE(instr->input(1), instr->input(2));

  if (!instr->output_type()->is<VoidType>()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }

  if (!instr->input(0)->type()->is<BoolType>()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0);
    return;
  }
  if (!instr->input(1)->is<BasicBlock>()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1);
    return;
  }
  if (!instr->input(2)->is<BasicBlock>()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 2);
    return;
  }
  if (instr->input(1) == instr->input(2)) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 2);
    return;
  }
}

void Validator::VisitCall(CallInstruction* instr) {
  auto const function_type = instr->input(0)->type()->as<FunctionType>();
  if (!function_type) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0);
    return;
  }
  if (instr->output_type() != function_type->return_type()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }
  if (instr->input(1)->type() != function_type->parameters_type()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1);
    return;
  }
}

void Validator::VisitElement(ElementInstruction* instr) {
  ValidateArrayAccess(instr);
}

void Validator::VisitEntry(EntryInstruction* instr) {
  if (instr->output_type() != instr->function()->parameters_type())
    Error(ErrorCode::ValidateInstructionOutput, instr);
}

void Validator::VisitExit(ExitInstruction* instr) {
  if (!instr->output_type()->is<VoidType>()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }
}

// 'get' instruction must be followed by another 'get' instruction which refers
// same tuple value or an instruction which yields tuple value.
void Validator::VisitGet(GetInstruction* instr) {
  auto const previous = instr->previous();
  if (!previous) {
    Error(ErrorCode::ValidateInstructionGet, instr);
    return;
  }

  auto const tuple_value = instr->input(0);
  auto const tuple_type = tuple_value->type()->as<TupleType>();
  if (!tuple_type) {
    Error(ErrorCode::ValidateInstructionType, instr, 0);
    return;
  }

  if (static_cast<size_t>(instr->index()) >= tuple_type->members().size()) {
    Error(ErrorCode::ValidateInstructionGet, instr);
    return;
  }

  if (tuple_value != previous &&
      (!previous->is<GetInstruction>() || tuple_value != previous->input(0))) {
    Error(ErrorCode::ValidateInstructionGet, instr);
    return;
  }
}

void Validator::VisitIf(IfInstruction* instr) {
  if (instr->output_type()->is<VoidType>()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }

  if (!instr->input(0)->type()->is<BoolType>()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0);
    return;
  }
  if (instr->input(1)->type() != instr->output_type()) {
    Error(ErrorCode::ValidateInstructionType, instr, 1);
    return;
  }
  if (instr->input(2)->type() != instr->output_type()) {
    Error(ErrorCode::ValidateInstructionType, instr, 2);
    return;
  }
}

void Validator::VisitJump(JumpInstruction* instr) {
  if (instr->input(0)->is<BasicBlock>())
    return;
  Error(ErrorCode::ValidateInstructionOperand, instr, 0);
}

void Validator::VisitLoad(LoadInstruction* instr) {
  auto const pointer_type = instr->input(0)->type()->as<PointerType>();
  if (!pointer_type) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0,
          instr->input(0)->type());
    return;
  }
  if (instr->output_type() != pointer_type->pointee()) {
    Error(ErrorCode::ValidateInstructionOutput, instr, pointer_type);
    return;
  }
}

void Validator::VisitPhi(PhiInstruction* instr) {
  for (auto const predecessor : instr->basic_block()->predecessors()) {
    if (!instr->FindPhiInputFor(predecessor)) {
      Error(ErrorCode::ValidatePhiNotFound, instr, predecessor);
      return;
    }
  }
  // TODO(eval1749) We should check type of `phi` operands are subtype of
  // output type.
  auto position = 0;
  for (auto const phi_input : instr->phi_inputs()) {
    if (phi_input->value()->type() != instr->output_type()) {
      Error(ErrorCode::ValidateInstructionOperand, instr,
            phi_input->basic_block());
      return;
    }
    ++position;
  }
  if (!position) {
    Error(ErrorCode::ValidatePhiCount, instr);
    return;
  }
}

void Validator::VisitRet(RetInstruction* instr) {
  auto const return_type = instr->function()->return_type();
  if (instr->input(0)->type() != return_type) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0, return_type);
    return;
  }
  auto const exit_block = instr->function()->exit_block();
  if (instr->input(1) != exit_block) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1, exit_block);
    return;
  }
}

void Validator::VisitStackAlloc(StackAllocInstruction* instr) {
  if (instr->count() >= 1)
    return;
  Error(ErrorCode::ValidateInstructionStackAlloc, instr);
}

void Validator::VisitStore(StoreInstruction* instr) {
  if (!instr->output_type()->is<VoidType>()) {
    Error(ErrorCode::ValidateInstructionOutput, instr);
    return;
  }
  auto const pointer_type = instr->input(0)->type()->as<PointerType>();
  if (!pointer_type) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0, pointer_type);
    return;
  }
  // TODO(eval1749) We should check type of instr->input(2) is subtype of
  // |pointer_type->pointee()|.
  auto const pointee = pointer_type->pointee();
  if (instr->input(1)->type() != pointee) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1, pointee);
    return;
  }
}

void Validator::VisitThrow(ThrowInstruction* instr) {
  if (instr->input(0)->is<VoidValue>()) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 0);
    return;
  }
  auto const exit_block = instr->function()->exit_block();
  if (instr->input(1) != exit_block) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1, exit_block);
    return;
  }
}

void Validator::VisitTuple(TupleInstruction* instr) {
  auto const tuple_type = instr->type()->as<TupleType>();
  if (!tuple_type) {
    Error(ErrorCode::ValidateInstructionTuple, instr);
    return;
  }
  auto const count = instr->CountInputs();
  if (!count) {
    Error(ErrorCode::ValidateInstructionTuple, instr);
    return;
  }
  auto index = 0;
  for (auto const type : tuple_type->members()) {
    if (instr->input(index)->type() != type)
      Error(ErrorCode::ValidateInstructionOperand, instr, 1, type);
    ++index;
  }
}

void Validator::VisitUnreachable(UnreachableInstruction* instr) {
  auto const exit_block = instr->function()->exit_block();
  if (instr->input(0) != exit_block) {
    Error(ErrorCode::ValidateInstructionOperand, instr, 1, exit_block);
    return;
  }
}

}  // namespace hir
}  // namespace elang
