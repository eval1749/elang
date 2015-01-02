// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/function_editor.h"

#include "base/logging.h"
#include "elang/hir/basic_block_editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// FunctionEditor
//
FunctionEditor::FunctionEditor(Factory* factory, Function* function)
    : function_(function), factory_(factory) {
  functions_.push_back(function_);
  if (!function->basic_blocks_.empty()) {
    DCHECK(Validate(function));
    return;
  }

  // Make entry and exit block
  auto const entry = factory->NewBasicBlock();
  auto const exit = factory->NewBasicBlock();
  function->basic_blocks_.AppendNode(entry);
  entry->id_ = ++function_->last_basic_block_id_;
  function->basic_blocks_.AppendNode(exit);
  exit->id_ = ++function_->last_basic_block_id_;

  // Set entry and exit instructions
  auto const void_type = factory->GetVoidType();
  auto const void_value = factory->GetVoidValue();
  BasicBlockEditor block(factory, exit);
  exit->function_ = function;
  block.AppendChild(ExitInstruction::New(factory, void_type));
  block.Edit(entry);
  entry->function_ = function;
  block.AppendChild(EntryInstruction::New(factory, void_type));
  block.AppendChild(block.NewReturn(void_value));

  DCHECK(Validate(function));
}

FunctionEditor::~FunctionEditor() {
#if _DEBUG
  for (auto const function : functions_) {
    DCHECK(Validate(function));
  }
#endif
}

bool FunctionEditor::Validate(Function* function) {
  if (function->basic_blocks().empty()) {
    DVLOG(0) << function << " should have blocks.";
    return false;
  }
  if (!function->entry_block()->first_instruction()->is<EntryInstruction>()) {
    DVLOG(0) << function << " should have an entry block.";
    return false;
  }
  auto found_exit = false;
  for (auto block : function->basic_blocks()) {
    if (!block->id()) {
      DVLOG(0) << block << " should have an id.";
      return false;
    }
    if (!BasicBlockEditor::Validate(block))
      return false;
    if (block->last_instruction()->is<ExitInstruction>()) {
      if (found_exit) {
        DVLOG(0) << function << " should have only one exit block.";
        return false;
      }
      found_exit = true;
    }
  }
  if (!found_exit) {
    DVLOG(0) << function << " should have an exit block.";
    return false;
  }
  return true;
}

}  // namespace hir
}  // namespace elang
