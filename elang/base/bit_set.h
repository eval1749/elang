// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_BIT_SET_H_
#define ELANG_BASE_BIT_SET_H_

#include <ostream>

#include "base/basictypes.h"
#include "elang/base/base_export.h"
#include "elang/base/zone_allocated.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// BitSet
//
class ELANG_BASE_EXPORT BitSet final : public ZoneAllocated {
 public:
  typedef uintptr_t Pack;

  class ELANG_BASE_EXPORT Iterator final {
   public:
    Iterator(const BitSet* bit_set, int index);
    Iterator(const Iterator& other);
    ~Iterator();

    Iterator& operator=(const Iterator& other);

    int operator*();
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    const BitSet* bit_set_;
    int index_;
  };

  BitSet(Zone* zone, const BitSet& other);
  BitSet(Zone* zone, int size);
  ~BitSet();

  Iterator begin() const;
  Iterator end() const;

  void Add(int index);
  void Clear();
  bool Contains(int index) const;
  void CopyFrom(const BitSet& other);
  bool Equals(const BitSet& other) const;
  void Intersect(const BitSet& other);
  bool IsEmpty() const;
  void Remove(int index);
  void Subtract(const BitSet& other);
  void Union(const BitSet& other);

 private:
  int IndexOf(int start) const;
  int LastIndexOf(int start) const;

  // Maximum number of bits in this |BitSet|.
  int const capacity_;
  // Number of allocated packs.
  int const pack_size_;
  // |packs_| should be initialized after |pack_size_|.
  Pack* const packs_;

  DISALLOW_COPY_AND_ASSIGN(BitSet);
};

ELANG_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                           const BitSet& simple_name);

}  // namespace elang

#endif  // ELANG_BASE_BIT_SET_H_
