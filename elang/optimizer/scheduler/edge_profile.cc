// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/optimizer/scheduler/edge_profile.h"

#include "elang/base/graphs/graph_sorter.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// EdgeProfile::Editor
//
EdgeProfile::Editor::Editor() : edge_profile_(new EdgeProfile()) {
}

EdgeProfile::Editor::~Editor() {
  DCHECK(!edge_profile_);
}

void EdgeProfile::Editor::Add(const BasicBlock* from,
                              const BasicBlock* to,
                              double value) {
  DCHECK_GE(value, 0);
  auto const key = std::make_pair(from, to);
  DCHECK(!edge_profile_->map_.count(key)) << from << "->" << to;
  edge_profile_->map_.insert(std::make_pair(key, value));
}

std::unique_ptr<EdgeProfile> EdgeProfile::Editor::Finish() {
  DCHECK(edge_profile_);
  return std::move(edge_profile_);
}

double EdgeProfile::Editor::FrequencyOf(const BasicBlock* from,
                                        const BasicBlock* to) const {
  return edge_profile_->FrequencyOf(from, to);
}

bool EdgeProfile::Editor::Has(const BasicBlock* from,
                              const BasicBlock* to) const {
  auto const it = edge_profile_->map_.find(std::make_pair(from, to));
  return it != edge_profile_->map_.end();
}

//////////////////////////////////////////////////////////////////////
//
// EdgeProfile
//
EdgeProfile::EdgeProfile() {
}

EdgeProfile::~EdgeProfile() {
}

double EdgeProfile::FrequencyOf(const BasicBlock* form,
                                const BasicBlock* to) const {
  auto const it = map_.find(std::make_pair(form, to));
  return it == map_.end() ? 0.0 : it->second;
}

}  // namespace optimizer
}  // namespace elang
