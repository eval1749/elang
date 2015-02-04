// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/lir/validator.h"

#include "base/logging.h"
#include "elang/lir/editor.h"
#include "elang/lir/error_code.h"
#include "elang/lir/error_data.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_map.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// Validator
//
Validator::Validator(Editor* editor) : editor_(editor) {
}

Validator::~Validator() {
}

void Validator::AddError(ErrorCode error_code,
                         Value value,
                         const std::vector<Value> details) {
  editor_->AddError(error_code, value, details);
}

Value Validator::AsValue(Instruction* instruction) {
  return editor_->factory()->literals()->RegisterInstruction(instruction);
}

void Validator::Error(ErrorCode error_code, Instruction* instruction) {
  Error(error_code, AsValue(instruction));
}

void Validator::Error(ErrorCode error_code, Value value) {
  AddError(error_code, value, {});
}

void Validator::Error(ErrorCode error_code, Value value, Value detail) {
  AddError(error_code, value, {detail});
}

void Validator::Error(ErrorCode error_code,
                      Value value,
                      Value detail1,
                      Value detail2) {
  AddError(error_code, value, {detail1, detail2});
}

bool Validator::Validate(BasicBlock* block) {
  if (!block->id()) {
    Error(ErrorCode::ValidateBasicBlockId, block->value());
    return false;
  }
  if (!block->function()) {
    Error(ErrorCode::ValidateBasicBlockFunction, block->value());
    return false;
  }
  if (block->instructions().empty()) {
    Error(ErrorCode::ValidateBasicBlockEmpty, block->value());
    return false;
  }
  auto found_terminator = false;
  for (auto instruction : block->instructions()) {
    // TODO(eval1749) We should call |Validation()| function in ISA.
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
  }
  if (!found_terminator) {
    Error(ErrorCode::ValidateBasicBlockTerminator, block->value());
    return false;
  }
  return true;
}

bool Validator::Validate(Function* function) {
  if (function->basic_blocks().empty()) {
    Error(ErrorCode::ValidateFunctionEmpty, function->value());
    return false;
  }
  if (!function->entry_block()->first_instruction()->is<EntryInstruction>()) {
    Error(ErrorCode::ValidateFunctionEntry, function->value());
    return false;
  }
  auto found_exit = false;
  auto is_valid = true;
  for (auto block : function->basic_blocks()) {
    if (!Validate(block)) {
      is_valid = false;
      continue;
    }
    if (block->last_instruction()->is<ExitInstruction>()) {
      if (found_exit) {
        Error(ErrorCode::ValidateBasicBlockExit, block->value());
        is_valid = false;
      }
      found_exit = true;
    }
  }
  if (!found_exit) {
    Error(ErrorCode::ValidateBasicBlockExit, function->value());
    is_valid = false;
  }
  return is_valid;
}

}  // namespace lir
}  // namespace elang
