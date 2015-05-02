// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/optimizer/scheduler/edge_profile_editor.h"

#include "base/logging.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// EdgeProfileEditor
//
EdgeProfileEditor::EdgeProfileEditor() : edge_profile_(new EdgeProfile()) {
}

EdgeProfileEditor::~EdgeProfileEditor() {
  DCHECK(!edge_profile_);
}

void EdgeProfileEditor::Add(const BasicBlock* from,
                            const BasicBlock* to,
                            double value) {
  DCHECK_GE(value, 0);
  auto const key = std::make_pair(from, to);
  DCHECK(!edge_profile_->map_.count(key)) << from << "->" << to;
  edge_profile_->map_.insert(std::make_pair(key, value));
}

std::unique_ptr<EdgeProfile> EdgeProfileEditor::Finish() {
  DCHECK(edge_profile_);
  return std::move(edge_profile_);
}

double EdgeProfileEditor::FrequencyOf(const BasicBlock* from,
                                      const BasicBlock* to) const {
  return edge_profile_->FrequencyOf(from, to);
}

bool EdgeProfileEditor::Has(const BasicBlock* from,
                            const BasicBlock* to) const {
  auto const it = edge_profile_->map_.find(std::make_pair(from, to));
  return it != edge_profile_->map_.end();
}

}  // namespace optimizer
}  // namespace elang
