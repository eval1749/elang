// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_USAGE_TRACKER_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_USAGE_TRACKER_H_

#include <vector>

#include "base/macros.h"
#include "elang/lir/analysis/use_def_list.h"
#include "elang/lir/lir_export.h"

namespace elang {

template <typename Graph>
class DominatorTree;

namespace lir {

class Editor;
class Function;
class Instruction;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// RegisterUsageTracker assigns index to instructions and build use-def list.
// for virtual registers.
//
class ELANG_LIR_EXPORT RegisterUsageTracker final {
 public:
  explicit RegisterUsageTracker(Editor* editor);
  ~RegisterUsageTracker();

  // Returns true if |input| is used after |instruction|.
  bool IsUsedAfter(Value input, Instruction* instruction) const;

  // Returns next user of |input| after |instruction|.
  Instruction* NextUseAfter(Value input, Instruction* instruction) const;

 private:
  const DominatorTree<Function>& dominator_tree_;
  const DominatorTree<Function>& post_dominator_tree_;
  const UseDefList use_def_list_;

  DISALLOW_COPY_AND_ASSIGN(RegisterUsageTracker);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_USAGE_TRACKER_H_
