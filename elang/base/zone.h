// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ZONE_H_
#define ELANG_BASE_ZONE_H_

#include "base/macros.h"
#include "elang/base/base_export.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// Zone
//
class ELANG_BASE_EXPORT Zone final {
 public:
  Zone(const Zone& other) = delete;
  Zone(Zone&& other);
  Zone();
  ~Zone();

  Zone& operator=(const Zone& other) = delete;
  Zone& operator=(Zone&& other);

  // Allocate |size| bytes of memory in the Zone.
  void* Allocate(size_t size);

  template <typename T>
  T* AllocateObjects(size_t length) {
    return static_cast<T*>(Allocate(length * sizeof(T)));
  }

 private:
  class Segment;

  Segment* segment_;
};

}  // namespace elang

#endif  // ELANG_BASE_ZONE_H_
