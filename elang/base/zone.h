// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ZONE_H_
#define ELANG_BASE_ZONE_H_

#include "base/macros.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// Zone
//
class Zone final {
 public:
  Zone();
  ~Zone();

  // Allocate |size| bytes of memory in the Zone.
  void* Allocate(size_t size);

 private:
  class Segment;

  Segment* segment_;

  DISALLOW_COPY_AND_ASSIGN(Zone);
};

}  // namespace elang

#endif  // ELANG_BASE_ZONE_H_
