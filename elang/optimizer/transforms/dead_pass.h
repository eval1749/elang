// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TRANSFORMS_DEAD_PASS_H_
#define ELANG_OPTIMIZER_TRANSFORMS_DEAD_PASS_H_

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
// DeadPass eliminates useless nodes from node graph.
//
class ELANG_OPTIMIZER_EXPORT DeadPass final : public api::Pass {
 public:
  DeadPass(api::PassController* pass_controller, Editor* editor);
  ~DeadPass();

  void Run();

 private:
  // api::Pass
  base::StringPiece name() const final;
  void DumpAfterPass(const api::PassDumpContext& context) final;
  void DumpBeforePass(const api::PassDumpContext& context) final;

  Editor& editor_;

  DISALLOW_COPY_AND_ASSIGN(DeadPass);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TRANSFORMS_DEAD_PASS_H_
