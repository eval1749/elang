// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ZONE_OWNER_H_
#define ELANG_BASE_ZONE_OWNER_H_

#include "base/macros.h"
#include "elang/base/base_export.h"
#include "elang/base/zone.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// ZoneOwner
//
class ELANG_BASE_EXPORT ZoneOwner {
 public:
  ZoneOwner(const ZoneOwner& other) = delete;

  ZoneOwner& operator=(const ZoneOwner& other) = delete;
  ZoneOwner& operator=(ZoneOwner&& other);

  Zone* zone() { return &zone_; }

  // Allocate |size| bytes of memory in the Zone.
  void* Allocate(size_t size);

 protected:
  ZoneOwner(ZoneOwner&& other);
  ZoneOwner();
  ~ZoneOwner();

 private:
  Zone zone_;
};

}  // namespace elang

#endif  // ELANG_BASE_ZONE_OWNER_H_
