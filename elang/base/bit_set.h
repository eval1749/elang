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
  class ELANG_BASE_EXPORT Iterator final {
   public:
    Iterator(uintptr_t* pointer, int index);
    Iterator(const Iterator& other);
    ~Iterator();

    Iterator& operator=(const Iterator& other);

    bool operator*();
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    uintptr_t* pointer_;
    int index_;
  };

  BitSet(Zone* zone, const BitSet& other);
  BitSet(Zone* zone, int size);
  ~BitSet() = delete;

  Iterator begin() const;
  Iterator end() const;
  int size() const { return size_; }

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
  int const data_size_;
  int const size_;
  uintptr_t* const data_;

  DISALLOW_COPY_AND_ASSIGN(BitSet);
};

ELANG_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                           const BitSet& simple_name);

}  // namespace elang

#endif  // ELANG_BASE_BIT_SET_H_
