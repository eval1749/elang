// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ANALYSIS_CONFLICT_MAP_BUILDER_H_
#define ELANG_LIR_ANALYSIS_CONFLICT_MAP_BUILDER_H_

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {

class BitSet;
template <typename Block, typename Value>
class LivenessCollection;

namespace lir {

class BasicBlock;
class ConflictMap;
class Editor;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// ConflictMapBuilder
//
class ELANG_LIR_EXPORT ConflictMapBuilder final {
 public:
  explicit ConflictMapBuilder(const Editor* editor);
  ~ConflictMapBuilder();

  ConflictMap Build();

 private:
  void UpdateConflictMapFromLiveness(ConflictMap* conflict_map,
                                     const BitSet& lives);

  const Editor* const editor_;
  const LivenessCollection<BasicBlock*, Value>& liveness_map_;

  DISALLOW_COPY_AND_ASSIGN(ConflictMapBuilder);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ANALYSIS_CONFLICT_MAP_BUILDER_H_
