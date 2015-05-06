// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_CLEAN_PASS_H_
#define ELANG_LIR_TRANSFORMS_CLEAN_PASS_H_

#include "base/macros.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/pass.h"

namespace elang {
namespace lir {

class Editor;

//////////////////////////////////////////////////////////////////////
//
// CleanPass eliminates useless control flow based on algorithm described in:
//  Engineering Aa Compiler, Second edition
//  Keith D. Cooper, Linda Torczon
//  February 2011
//
// |CleanPass| does following optimizations:
//   1 Fold a redundant branch
//   2 Remove an empty block
//   3 Combine blocks
//   4 Hoist branch
//
class ELANG_LIR_EXPORT CleanPass final : public FunctionPass {
 public:
  CleanPass(base::StringPiece name, Editor* editor);
  ~CleanPass() final;

 private:
  void Clean();
  void CleanBranch(BranchInstruction* instr);
  void CleanJump(JumpInstruction* instr);
  void DidChangeControlFlow(base::StringPiece message,
                            const Instruction* instr);
  void WillChangeControlFlow(base::StringPiece message,
                             const Instruction* instr);

  // FunctionPas
  void RunOnFunction() final;

  bool changed_;

  DISALLOW_COPY_AND_ASSIGN(CleanPass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_CLEAN_PASS_H_
