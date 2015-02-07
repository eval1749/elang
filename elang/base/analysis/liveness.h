// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ANALYSIS_LIVENESS_H_
#define ELANG_BASE_ANALYSIS_LIVENESS_H_

#include <ostream>

#include "base/macros.h"
#include "elang/base/bit_set.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/base_export.h"

namespace elang {

class LivenessEditorBase;

//////////////////////////////////////////////////////////////////////
//
// Liveness
//
class ELANG_BASE_EXPORT Liveness : public ZoneAllocated {
 public:
  ~Liveness() = delete;

  const BitSet& in() const { return in_; }
  const BitSet& kill() const { return kill_; }
  const BitSet& out() const { return out_; }

 private:
  friend class LivenessEditorBase;

  Liveness(Zone* zone, int size);

  BitSet in_;
  BitSet kill_;
  BitSet out_;

  DISALLOW_COPY_AND_ASSIGN(Liveness);
};

ELANG_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                           const Liveness& liveness);

}  // namespace elang

#endif  // ELANG_BASE_ANALYSIS_LIVENESS_H_
