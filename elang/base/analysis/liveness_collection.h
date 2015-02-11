// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_LIVENESS_COLLECTION_H_
#define ELANG_BASE_ANALYSIS_LIVENESS_COLLECTION_H_

#include <unordered_map>

#include "base/macros.h"
#include "base/logging.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"


namespace elang {

class BitSet;

template <typename Node, typename Variable>
class LivenessBuilder;

template <typename Node, typename Variable>
class LivenessEditor;

class Liveness;

//////////////////////////////////////////////////////////////////////
//
// LivenessCollection
//
template <typename Node, typename Variable>
class LivenessCollection final : public ZoneOwner {
 public:
  ~LivenessCollection() = default;

  const Liveness& LivenessOf(Node node) const {
    auto const it = node_map_.find(node);
    DCHECK(it != node_map_.end());
    return *it->second;
  }

  int NumberOf(Variable value) const {
    auto const it = variable_map_.find(value);
    return it == variable_map_.end() ? -1 : it->second;
  }

  Variable VariableOf(int number) const {
    return variables_[number];
  }

 private:
  friend class LivenessBuilder<Node, Variable>;
  friend class LivenessEditor<Node, Variable>;

  LivenessCollection()
      : node_map_(zone()), variable_map_(zone()), variables_(zone()),
        work_(nullptr) {}

  BitSet* work() const { return work_; }

  Liveness* EditableLivenessOf(Node node) const {
    auto const it = node_map_.find(node);
    DCHECK(it != node_map_.end());
    return it->second;
  }

  ZoneUnorderedMap<Node, Liveness*> node_map_;
  ZoneUnorderedMap<Variable, int> variable_map_;
  ZoneVector<Variable> variables_;

  // Work area for backward solver.
  BitSet* work_;

  DISALLOW_COPY_AND_ASSIGN(LivenessCollection);
};

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_LIVENESS_COLLECTION_H_
