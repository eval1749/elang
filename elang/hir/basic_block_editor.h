// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_BASIC_BLOCK_EDITOR_H_
#define ELANG_HIR_BASIC_BLOCK_EDITOR_H_

#include <vector>

#include "elang/hir/hir_export.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

class ReturnInstruction;

//////////////////////////////////////////////////////////////////////
//
// BasicBlockEditor
//
class ELANG_HIR_EXPORT BasicBlockEditor final {
 public:
  explicit BasicBlockEditor(Factory* factory, BasicBlock* basic_block);
  ~BasicBlockEditor();

  void AppendChild(Instruction* instruction);
  void Edit(BasicBlock* basic_block);
  ReturnInstruction* NewReturn(Value* value);
  static bool Validate(BasicBlock* basic_block);

 private:
  BasicBlock* basic_block_;
  std::vector<BasicBlock*> basic_blocks_;
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlockEditor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_BASIC_BLOCK_EDITOR_H_
