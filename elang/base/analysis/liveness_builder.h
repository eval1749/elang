// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_LIVENESS_BUILDER_H_
#define ELANG_BASE_ANALYSIS_LIVENESS_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "elang/base/base_export.h"
#include "elang/base/analysis/liveness.h"
#include "elang/base/analysis/liveness_collection.h"
#include "elang/base/analysis/liveness_editor.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// LivenessBuilder
//
template <typename Node, typename Variable>
class LivenessBuilder : public LivenessEditor<Node, Variable> {
 public:
  typedef LivenessCollection<Node, Variable> Collection;

  LivenessBuilder()
      : LivenessEditor(new Collection()), collection_(collection()) {}
  ~LivenessBuilder() = default;

  void AddNode(Node node) {
    DCHECK(!collection_->node_map_.count(node));
    collection_->node_map_[node] = NewLiveness(zone(), bit_set_size());
  }

  void AddVariable(Variable value) {
    DCHECK(!collection_->variable_map_.count(value));
    auto const number = static_cast<int>(collection_->variables_.size());
    collection_->variables_.push_back(value);
    collection_->variable_map_[value] = number;
  }

  // Returns newly created liveness collection based on given information so
  // far.
  std::unique_ptr<Collection> Finish() {
    collection_->work_ = new (zone()) BitSet(zone(), bit_set_size());
    return std::move(collection_);
  }

  void MarkKill(Liveness* liveness, Variable value) {
    LivenessEditor::MarkKill(liveness, NumberOf(value));
  }

  void MarkUse(Liveness* liveness, Variable value) {
    LivenessEditor::MarkUse(liveness, NumberOf(value));
  }

  int NumberOf(Variable value) const { return collection_->NumberOf(value); }

 private:
  int bit_set_size() {
    return static_cast<int>(collection_->variable_map_.size());
  }

  Zone* zone() const { return collection_->zone(); }

  std::unique_ptr<Collection> collection_;

  DISALLOW_COPY_AND_ASSIGN(LivenessBuilder);
};

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_LIVENESS_BUILDER_H_
