// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/zone_allocated.h"

#include "base/logging.h"
#include "elang/base/zone.h"

namespace elang {

ZoneAllocated::ZoneAllocated() {
}

ZoneAllocated::~ZoneAllocated() {
  NOTREACHED();
}

void ZoneAllocated::operator delete(void* pointer, Zone* zone) {
  __assume(pointer);
  __assume(zone);
  NOTREACHED();
}

void* ZoneAllocated::operator new(size_t size, Zone* zone) {
  return zone->Allocate(size);
}

}  // namespace elang
