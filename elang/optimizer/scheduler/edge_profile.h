// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_EDGE_PROFILE_H_
#define ELANG_OPTIMIZER_SCHEDULER_EDGE_PROFILE_H_

#include <map>
#include <memory>
#include <utility>

#include "base/macros.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class EdgeProfileEditor;

//////////////////////////////////////////////////////////////////////
//
// EdgeProfile
//
class EdgeProfile final {
 public:
  using Edge = std::pair<const BasicBlock*, const BasicBlock*>;
  using Map = std::map<Edge, double>;

  EdgeProfile();
  ~EdgeProfile();

  const Map& all_edges() const { return map_; }

  double FrequencyOf(const BasicBlock* form, const BasicBlock* to) const;

 private:
  friend class EdgeProfileEditor;

  Map map_;

  DISALLOW_COPY_AND_ASSIGN(EdgeProfile);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_EDGE_PROFILE_H_
