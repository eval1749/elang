// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TRANSFORMS_CLEAN_PASS_H_
#define ELANG_OPTIMIZER_TRANSFORMS_CLEAN_PASS_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/api/pass.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class Editor;
class Node;

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
//
// |CleanPass| doesn't do "Hoist a branch", because our IR doesn't have
// empty branch block and it is complex to update 'phi' instructions.
//
class ELANG_OPTIMIZER_EXPORT CleanPass final : public api::Pass {
 public:
  CleanPass(api::PassObserver* observer, Editor* editor);
  ~CleanPass();

  void Run();

 private:
  void Clean();
  void CleanIf(Node* node);
  void CleanJump(Node* node);
  void DidChangeControlFlow(base::StringPiece message, const Node* last_node);
  void WillChangeControlFlow(base::StringPiece message, const Node* last_node);

  // api::Pass
  base::StringPiece name() const final;
  void DumpAfterPass(const api::PassDumpContext& context) final;
  void DumpBeforePass(const api::PassDumpContext& context) final;

  bool changed_;
  Editor& editor_;

  DISALLOW_COPY_AND_ASSIGN(CleanPass);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TRANSFORMS_CLEAN_PASS_H_
