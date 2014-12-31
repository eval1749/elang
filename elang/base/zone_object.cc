// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/zone_object.h"

#include "base/logging.h"
#include "elang/base/zone.h"

namespace elang {

ZoneObject::ZoneObject() {
}

ZoneObject::~ZoneObject() {
  NOTREACHED();
}

void ZoneObject::operator delete(void* pointer, Zone* zone) {
  __assume(pointer);
  __assume(zone);
  NOTREACHED();
}

void* ZoneObject::operator new (size_t size, Zone* zone) {
  return zone->Allocate(size);
}

}  // namespace elang
