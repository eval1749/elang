// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_LIVENESS_EDITOR_H_
#define ELANG_BASE_ANALYSIS_LIVENESS_EDITOR_H_

#include <memory>

#include "base/macros.h"
#include "elang/base/base_export.h"
#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"

namespace elang {

class BitSet;

//////////////////////////////////////////////////////////////////////
//
// LivenessEditor
//
class ELANG_BASE_EXPORT LivenessEditorBase {
 protected:
  LivenessEditorBase();
  ~LivenessEditorBase();

  BitSet* EditIn(Liveness* liveness) { return &liveness->in_; }
  BitSet* EditKill(Liveness* liveness) { return &liveness->kill_; }
  BitSet* EditOut(Liveness* liveness) { return &liveness->out_; }

  void MarkKill(Liveness* liveness, int number);
  void MarkUse(Liveness* liveness, int number);
  Liveness* NewLiveness(Zone* zone, int size);

 private:
  DISALLOW_COPY_AND_ASSIGN(LivenessEditorBase);
};

//////////////////////////////////////////////////////////////////////
//
// LivenessEditor
//
template <typename Block, typename Value>
class LivenessEditor : public LivenessEditorBase {
 public:
  typedef LivenessCollection<Block, Value> Collection;

  Liveness* Edit(Block block) const {
    return collection_->EditableLivenessOf(block);
  }

  const Liveness& LivenessOf(Block block) const {
    return collection_->LivenessOf(block);
  }

 protected:
  explicit LivenessEditor(Collection* collection) : collection_(collection) {}
  ~LivenessEditor() = default;

  Collection* collection() const { return collection_; }
  BitSet* work() const { return collection()->work_; }

 private:
  Collection* const collection_;

  DISALLOW_COPY_AND_ASSIGN(LivenessEditor);
};

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_LIVENESS_EDITOR_H_
