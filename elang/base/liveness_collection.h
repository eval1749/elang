// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_LIVENESS_COLLECTION_H_
#define ELANG_BASE_LIVENESS_COLLECTION_H_

#include <unordered_map>

#include "base/macros.h"
#include "base/logging.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"

namespace elang {

template <typename Block, typename Value>
class LivenessBuilder;
class Liveness;

//////////////////////////////////////////////////////////////////////
//
// LivenessCollection
//
template <typename Block, typename Value>
class LivenessCollection final : public ZoneOwner {
 public:
  ~LivenessCollection() = default;

  const Liveness& LivenessOf(Block block) const {
    auto const it = block_map_.find(block);
    DCHECK(it != block_map_.end());
    return *it->second;
  }

  int NumberOf(Value value) const {
    auto const it = value_map_.find(value);
    return it == value_map_.end() ? -1 : it->second;
  }

 private:
  friend class LivenessBuilder<Block, Value>;

  LivenessCollection() : block_map_(zone()), value_map_(zone()) {}

  ZoneUnorderedMap<Block, Liveness*> block_map_;
  ZoneUnorderedMap<Value, int> value_map_;

  DISALLOW_COPY_AND_ASSIGN(LivenessCollection);
};

}  // namespace elang

#endif  // ELANG_BASE_LIVENESS_COLLECTION_H_
