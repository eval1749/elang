// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/zone_owner.h"

namespace elang {

ZoneOwner::ZoneOwner() {
}

ZoneOwner::~ZoneOwner() {
}

void* ZoneOwner::Allocate(size_t size) {
  return zone()->Allocate(size);
}

}  // namespace elang
