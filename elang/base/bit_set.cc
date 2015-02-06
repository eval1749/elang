// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/bit_set.h"

#include "base/logging.h"
#include "elang/base/zone.h"

namespace elang {

namespace {
const uintptr_t kOne = static_cast<uintptr_t>(1);
const int kBitSize = sizeof(uintptr_t) == 8 ? 6 : 5;
uintptr_t kShiftMask = (kOne << kBitSize) - 1;

int IndexOf(int index) {
  return (index + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
}

int ShiftCountOf(int index) {
  return index & kShiftMask;
}

uintptr_t BitMaskOf(int index) {
  return kOne << ShiftCountOf(index);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// BitSet::Iterator
//
BitSet::Iterator::Iterator(uintptr_t* pointer, int index)
    : index_(index), pointer_(pointer) {
}

BitSet::Iterator::Iterator(const Iterator& other)
    : index_(other.index_), pointer_(other.pointer_) {
}

BitSet::Iterator::~Iterator() {
}

bool BitSet::Iterator::operator*() {
  return (pointer_[IndexOf(index_)] & BitMaskOf(index_)) != 0;
}

BitSet::Iterator& BitSet::Iterator::operator++() {
  ++index_;
  return *this;
}

bool BitSet::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(pointer_, other.pointer_);
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
    : data_size_(other.data_size_),
      size_(other.size_),
      data_(zone->AllocateObjects<uintptr_t>(data_size_)) {
  CopyFrom(other);
}

BitSet::BitSet(Zone* zone, int size)
    : data_size_(IndexOf(size)),
      size_(size),
      data_(zone->AllocateObjects<uintptr_t>(data_size_)) {
  DCHECK_GT(size_, 0);
  Clear();
}

BitSet::Iterator BitSet::begin() const {
  return Iterator(data_, 0);
}

BitSet::Iterator BitSet::end() const {
  return Iterator(data_, size_);
}

void BitSet::Add(int index) {
  data_[IndexOf(index)] |= BitMaskOf(index);
}

void BitSet::Clear() {
  for (auto i = 0; i < data_size_; ++i)
    data_[i] = 0;
}

bool BitSet::Contains(int index) const {
  return (data_[IndexOf(index)] & BitMaskOf(index)) != 0;
}

void BitSet::CopyFrom(const BitSet& other) {
  DCHECK_GE(size_, other.size_);
  for (int i = 0; i < other.data_size_; ++i)
    data_[i] = other.data_[i];
  for (int i = other.data_size_; i < data_size_; ++i)
    data_[i] = 0;
}

bool BitSet::Equals(const BitSet& other) const {
  DCHECK_EQ(size_, other.size_);
  for (auto index = 0; index < data_size_; ++index) {
    if (data_[index] != other.data_[index])
      return false;
  }
  return true;
}

void BitSet::Intersect(const BitSet& other) {
  DCHECK_EQ(size_, other.size_);
  for (auto i = 0; i < data_size_; ++i)
    data_[i] &= other.data_[i];
}

bool BitSet::IsEmpty() const {
  for (auto i = 0; i < data_size_; ++i) {
    if (data_[i])
      return false;
  }
  return true;
}

void BitSet::Remove(int index) {
  data_[IndexOf(index)] &= ~BitMaskOf(index);
}

void BitSet::Subtract(const BitSet& other) {
  DCHECK_EQ(size_, other.size_);
  for (auto i = 0; i < data_size_; ++i)
    data_[i] &= ~other.data_[i];
}

void BitSet::Union(const BitSet& other) {
  DCHECK_EQ(size_, other.size_);
  for (auto i = 0; i < data_size_; ++i)
    data_[i] |= other.data_[i];
}

std::ostream& operator<<(std::ostream& ostream, const BitSet& bit_set) {
  ostream << "{";
  auto separator = "";
  for (auto position = 0; position < bit_set.size(); ++position) {
    if (!bit_set.Contains(position))
      continue;
    ostream << separator << position;
    separator = ", ";
  }
  return ostream << "}";
}

}  // namespace elang
