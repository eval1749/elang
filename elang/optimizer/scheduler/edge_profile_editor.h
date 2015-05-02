// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_EDGE_PROFILE_EDITOR_H_
#define ELANG_OPTIMIZER_SCHEDULER_EDGE_PROFILE_EDITOR_H_

#include <memory>

#include "base/macros.h"
#include "elang/optimizer/scheduler/edge_profile.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class EdgeProfile;

//////////////////////////////////////////////////////////////////////
//
// EdgeProfileEditor
//
class EdgeProfileEditor final {
 public:
  using Map = EdgeProfile::Map;

  EdgeProfileEditor();
  ~EdgeProfileEditor();

  const Map& all_edges() const { return edge_profile_->all_edges(); }

  void Add(const BasicBlock* from, const BasicBlock* to, double frequency);
  std::unique_ptr<EdgeProfile> Finish();
  double FrequencyOf(const BasicBlock* form, const BasicBlock* to) const;
  bool Has(const BasicBlock* from, const BasicBlock* to) const;

 private:
  std::unique_ptr<EdgeProfile> edge_profile_;

  DISALLOW_COPY_AND_ASSIGN(EdgeProfileEditor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_EDGE_PROFILE_EDITOR_H_
