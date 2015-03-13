// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REMOVE_CRITICAL_EDGES_PASS_H_
#define ELANG_LIR_TRANSFORMS_REMOVE_CRITICAL_EDGES_PASS_H_

#include "elang/lir/lir_export.h"
#include "elang/lir/pass.h"

namespace elang {
namespace lir {

struct Value;

//////////////////////////////////////////////////////////////////////
//
// RemoveCriticalEdgesPass removes critical edges between predecessors which
// have multiple successors and a block having phi-instruction, or back edge
// from multiple successors.
//
class ELANG_LIR_EXPORT RemoveCriticalEdgesPass final : public FunctionPass {
 public:
  explicit RemoveCriticalEdgesPass(Editor* editor);
  ~RemoveCriticalEdgesPass();

 private:
  // Pass
  base::StringPiece name() const final;

  // FunctionPass
  void RunOnFunction() final;

  DISALLOW_COPY_AND_ASSIGN(RemoveCriticalEdgesPass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REMOVE_CRITICAL_EDGES_PASS_H_
