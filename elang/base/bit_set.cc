// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/bit_set.h"

#include "base/logging.h"
#include "elang/base/bits.h"
#include "elang/base/zone.h"

namespace elang {

namespace {

const BitSet::Pack kOne = static_cast<BitSet::Pack>(1);
const int kPackSize = sizeof(BitSet::Pack) * 8;
const int kShiftCount = sizeof(BitSet::Pack) == 8 ? 6 : 5;
const int kShiftMask = (1 << kShiftCount) - 1;

int PackIndexOf(int index) {
  return index >> kShiftCount;
}

int ShiftCountOf(int index) {
  return index & kShiftMask;
}

BitSet::Pack BitMaskOf(int index) {
  return kOne << ShiftCountOf(index);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// BitSet::Iterator
//
BitSet::Iterator::Iterator(const BitSet* bit_set, int index)
    : bit_set_(bit_set), index_(index) {
  DCHECK_GE(index_, -1);
  DCHECK_LE(index_, bit_set_->size_);
}

BitSet::Iterator::Iterator(const Iterator& other)
    : Iterator(other.bit_set_, other.index_) {
}

BitSet::Iterator::~Iterator() {
}

int BitSet::Iterator::operator*() {
  DCHECK_GE(index_, 0);
  DCHECK_LT(index_, bit_set_->size());
  return index_;
}

BitSet::Iterator& BitSet::Iterator::operator++() {
  DCHECK_GE(index_, 0);
  DCHECK_LE(index_ + 1, bit_set_->size());
  index_ = bit_set_->IndexOf(index_ + 1);
  return *this;
}

bool BitSet::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(bit_set_, other.bit_set_);
  return index_ == other.index_;
}

bool BitSet::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// BitSet
//
BitSet::BitSet(Zone* zone, const BitSet& other)
    : pack_size_(other.pack_size_),
      size_(other.size_),
      packs_(zone->AllocateObjects<Pack>(pack_size_)) {
  CopyFrom(other);
}

BitSet::~BitSet() {
  NOTREACHED();
}

BitSet::BitSet(Zone* zone, int size)
    : pack_size_((size + kPackSize - 1) / kPackSize),
      size_(size),
      packs_(zone->AllocateObjects<Pack>(pack_size_)) {
  DCHECK_GT(size_, 0);
  Clear();
}

BitSet::Iterator BitSet::begin() const {
  return Iterator(this, IndexOf(0));
}

BitSet::Iterator BitSet::end() const {
  DCHECK_GT(size_, 0);
  return Iterator(this, LastIndexOf(size_ - 1));
}

void BitSet::Add(int index) {
  DCHECK_NE(BitMaskOf(index), 0u);
  auto const pack_index = PackIndexOf(index);
  DCHECK_LT(pack_index, pack_size_);
  packs_[pack_index] |= BitMaskOf(index);
}

void BitSet::Clear() {
  for (auto i = 0; i < pack_size_; ++i)
    packs_[i] = 0;
}

bool BitSet::Contains(int index) const {
  return (packs_[PackIndexOf(index)] & BitMaskOf(index)) != 0;
}

void BitSet::CopyFrom(const BitSet& other) {
  DCHECK_GE(size_, other.size_);
  for (int i = 0; i < other.pack_size_; ++i)
    packs_[i] = other.packs_[i];
  for (int i = other.pack_size_; i < pack_size_; ++i)
    packs_[i] = 0;
}

bool BitSet::Equals(const BitSet& other) const {
  DCHECK_EQ(size_, other.size_);
  for (auto index = 0; index < pack_size_; ++index) {
    if (packs_[index] != other.packs_[index])
      return false;
  }
  return true;
}

// Returns an index where |packs_[index] == 1| from |start| until |size_| or
// |size_| if there are no one in |data|.
int BitSet::IndexOf(int start) const {
  DCHECK_GE(start, 0);
  DCHECK_LE(start, size_);
  if (start == size_)
    return size_;
  auto index = start;
  auto pack_index = PackIndexOf(index);
  auto pack = packs_[pack_index] >> ShiftCountOf(index);
  if (!pack) {
    do {
      ++pack_index;
      if (pack_index == pack_size_)
        return -1;
      pack = packs_[pack_index];
    } while (!pack);
    index = pack_index * kPackSize;
  }
  // We found pack which contains one, let's calculate index of one bit.
  DCHECK_NE(pack, 0);
  index += CountTrailingZeros(pack);
  DCHECK_LE(index, size_);
  return index;
}

void BitSet::Intersect(const BitSet& other) {
  DCHECK_EQ(size_, other.size_);
  for (auto i = 0; i < pack_size_; ++i)
    packs_[i] &= other.packs_[i];
}

bool BitSet::IsEmpty() const {
  for (auto i = 0; i < pack_size_; ++i) {
    if (packs_[i])
      return false;
  }
  return true;
}

// Returns an last index where bit is one or |-1| if not found.
int BitSet::LastIndexOf(int start) const {
  DCHECK_GE(start, 0);
  DCHECK_LT(start, size_);
  auto index = start;
  auto pack_index = PackIndexOf(index);
  auto pack = packs_[pack_index] >> ShiftCountOf(start);
  if (!pack) {
    do {
      if (!pack_index)
        return -1;
      --pack_index;
      pack = packs_[pack_index];
    } while (!pack);
    index = pack_index * kPackSize;
  }
  // We found pack which contains one, let's calculate index of MSB + 1.
  DCHECK_NE(pack, 0);
  index += kPackSize - CountLeadingZeros(pack);
  DCHECK_LE(index, size_);
  return index;
}

void BitSet::Remove(int index) {
  packs_[PackIndexOf(index)] &= ~BitMaskOf(index);
}

void BitSet::Subtract(const BitSet& other) {
  DCHECK_EQ(size_, other.size_);
  for (auto i = 0; i < pack_size_; ++i)
    packs_[i] &= ~other.packs_[i];
}

void BitSet::Union(const BitSet& other) {
  DCHECK_EQ(size_, other.size_);
  for (auto i = 0; i < pack_size_; ++i)
    packs_[i] |= other.packs_[i];
}

std::ostream& operator<<(std::ostream& ostream, const BitSet& bit_set) {
  ostream << "{";
  auto separator = "";
  for (auto const index : bit_set) {
    ostream << separator << index;
    separator = ", ";
  }
  return ostream << "}";
}

}  // namespace elang
