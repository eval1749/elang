// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ZONE_OBJECT_H_
#define ELANG_BASE_ZONE_OBJECT_H_

#include "base/macros.h"

namespace elang {

class Zone;

//////////////////////////////////////////////////////////////////////
//
// ZoneObject
//
class ZoneObject {
 public:
  void* operator new(size_t size, Zone* zone);

  // |ZoneObject| can't have desturcotr and |delete| operator. But MSVC
  // requires them.
  ~ZoneObject();
  void operator delete(void*, Zone*);

 protected:
  ZoneObject();
};

}  // namespace elang

#endif  // ELANG_BASE_ZONE_OBJECT_H_
