// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/sequence_id_source.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// SequenceIdSource
//
SequenceIdSource::SequenceIdSource() : last_id_(0) {
}

SequenceIdSource::~SequenceIdSource() {
}

size_t SequenceIdSource::NextId() {
  return ++last_id_;
}

}  // namespace optimizer
}  // namespace elang
