// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ZONE_UNORDERED_SET_H_
#define ELANG_BASE_ZONE_UNORDERED_SET_H_

#include <unordered_set>

#include "elang/base/zone.h"
#include "elang/base/zone_allocator.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// ZoneUnorderedSet
// A wrapper subclass for |std::unordered_set|.
//
template <typename T>
class ZoneUnorderedSet
    : public std::unordered_set<T,
                                typename std::unordered_set<T>::hasher,
                                typename std::unordered_set<T>::key_equal,
                                ZoneAllocator<T>> {
  typedef std::unordered_set<T,
                             typename std::unordered_set<T>::hasher,
                             typename std::unordered_set<T>::key_equal,
                             ZoneAllocator<T>> BaseClass;

 public:
  explicit ZoneUnorderedSet(Zone* zone) : BaseClass(ZoneAllocator<T>(zone)) {}
};

}  // namespace elang

#endif  // ELANG_BASE_ZONE_UNORDERED_SET_H_
