// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/optimizer/scheduler/edge_profile.h"

namespace elang {
namespace optimizer {

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
