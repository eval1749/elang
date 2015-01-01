// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ZONE_UNORDERED_MAP_H_
#define ELANG_BASE_ZONE_UNORDERED_MAP_H_

#include <unordered_map>

#include "elang/base/zone.h"
#include "elang/base/zone_allocator.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// ZoneUnorderedMap
// A wrapper subclass for |std::vector|.
//
template <typename K, typename T>
class ZoneUnorderedMap
    : public std::unordered_map<K,
                                T,
                                typename std::unordered_map<K, T>::hasher,
                                typename std::unordered_map<K, T>::key_equal,
                                ZoneAllocator<T>> {
  typedef std::unordered_map<K,
                             T,
                             typename std::unordered_map<K, T>::hasher,
                             typename std::unordered_map<K, T>::key_equal,
                             ZoneAllocator<T>> BaseClass;

 public:
  explicit ZoneUnorderedMap(Zone* zone) : BaseClass(ZoneAllocator<T>(zone)) {}
};

}  // namespace elang

#endif  // ELANG_BASE_ZONE_UNORDERED_MAP_H_
