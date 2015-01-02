// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/basic_block_editor.h"

#include "base/logging.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// BasicBlockEditor
//
BasicBlockEditor::BasicBlockEditor(Factory* factory, BasicBlock* basic_block)
    : basic_block_(basic_block), factory_(factory) {
  basic_blocks_.push_back(basic_block_);
  DCHECK(Validate(basic_block));
}

BasicBlockEditor::~BasicBlockEditor() {
  for (auto const basic_block : basic_blocks_) {
    DCHECK(Validate(basic_block));
  }
}

void BasicBlockEditor::AppendChild(Instruction* new_instruction) {
  DCHECK(!new_instruction->basic_block_);
  new_instruction->set_id(++basic_block_->last_instruction_id_);
  new_instruction->basic_block_ = basic_block_;
  basic_block_->instructions_.AppendNode(new_instruction);
}

void BasicBlockEditor::Edit(BasicBlock* basic_block) {
  DCHECK(std::find(basic_blocks_.begin(), basic_blocks_.end(), basic_block) ==
         basic_blocks_.end());
  basic_blocks_.push_back(basic_block);
  basic_block_ = basic_block;
  DCHECK(Validate(basic_block));
}

ReturnInstruction* BasicBlockEditor::NewReturn(Value* value) {
  return ReturnInstruction::New(
      factory_, factory_->GetVoidType(), value,
      basic_block_->function_->basic_blocks().last_node());
}

bool BasicBlockEditor::Validate(BasicBlock* basic_block) {
  if (basic_block->instructions().empty()) {
    if (!basic_block->function_)
      return true;
    DVLOG(0) << basic_block << " is empty.";
    return false;
  }
  auto found_terminator = false;
  for (auto instruction : basic_block->instructions()) {
    if (!instruction->id()) {
      DVLOG(0) << instruction << " should have an id.";
      return false;
    }
    if (instruction->IsTerminator()) {
      if (found_terminator) {
        DVLOG(0) << basic_block << " has " << instruction << " at middle.";
        return false;
      }
      found_terminator = true;
    }
  }
  if (!found_terminator) {
    DVLOG(0) << basic_block << " should have terminator instruction"
             << " instead of " << basic_block->last_instruction();
    return false;
  }
  return true;
}

}  // namespace hir
}  // namespace elang
