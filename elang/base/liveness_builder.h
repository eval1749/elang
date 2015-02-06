// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_LIVENESS_BUILDER_H_
#define ELANG_BASE_LIVENESS_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "elang/base/base_export.h"
#include "elang/base/liveness.h"
#include "elang/base/liveness_collection.h"

namespace elang {

class BitSet;

//////////////////////////////////////////////////////////////////////
//
// LivenessEditor
//
class ELANG_BASE_EXPORT LivenessEditor {
 protected:
  LivenessEditor();
  ~LivenessEditor();

  BitSet* in_of(Liveness* liveness) { return &liveness->in_; }
  BitSet* kill_of(Liveness* liveness) { return &liveness->kill_; }
  BitSet* out_of(Liveness* liveness) { return &liveness->out_; }

  void MarkKill(Liveness* liveness, int number);
  void MarkUse(Liveness* liveness, int number);
  Liveness* NewLiveness(Zone* zone, int size);

 private:
  DISALLOW_COPY_AND_ASSIGN(LivenessEditor);
};

//////////////////////////////////////////////////////////////////////
//
// LivenessBuilder
//
template <typename Block, typename Value>
class LivenessBuilder : public LivenessEditor {
 public:
  typedef LivenessCollection<Block, Value> Collection;

  LivenessBuilder() : collection_(new Collection()) {}
  ~LivenessBuilder() = default;

  void AddBlock(Block block) {
    DCHECK(!collection_->block_map_.count(block));
    auto const size = static_cast<int>(collection_->value_map_.size());
    collection_->block_map_[block] = NewLiveness(zone(), size);
  }

  void AddValue(Value value) {
    DCHECK(!collection_->value_map_.count(value));
    collection_->value_map_[value] =
        static_cast<int>(collection_->value_map_.size());
  }

  std::unique_ptr<Collection> Finish() { return std::move(collection_); }

  Liveness* LivenessOf(Block block) {
    return &const_cast<Liveness&>(collection_->LivenessOf(block));
  }

  void MarkKill(Liveness* liveness, Value value) {
    LivenessEditor::MarkKill(liveness, NumberOf(value));
  }

  void MarkUse(Liveness* liveness, Value value) {
    LivenessEditor::MarkUse(liveness, NumberOf(value));
  }

  int NumberOf(Value value) const { return collection_->NumberOf(value); }

 private:
  Zone* zone() const { return collection_->zone(); }

  std::unique_ptr<Collection> collection_;

  DISALLOW_COPY_AND_ASSIGN(LivenessBuilder);
};

}  // namespace elang

#endif  // ELANG_BASE_LIVENESS_BUILDER_H_
