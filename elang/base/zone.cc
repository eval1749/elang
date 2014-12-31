// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/zone_allocated.h"

#include "elang/base/zone.h"

namespace elang {

namespace {
const size_t kAllocateUnit = 8;
const size_t kMinSegmentSize = 8 * 1024;

size_t RoundUp(size_t num, size_t unit) {
  return ((num + unit - 1) / unit) * unit;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Zone::Segment
//
class Zone::Segment final {
 public:
  Segment(size_t size, Segment* next);
  ~Segment();

  Segment* next() const { return next_; }

  void* Allocate(size_t size);

 private:
  Segment* const next_;
  size_t const max_offset_;
  size_t offset_;
  char* memory_;

  DISALLOW_COPY_AND_ASSIGN(Segment);
};

Zone::Segment::Segment(size_t size, Segment* next)
    : next_(next),
      max_offset_(RoundUp(size, kMinSegmentSize)),
      offset_(0u),
      memory_(new char[max_offset_]) {
}

Zone::Segment::~Segment() {
  delete memory_;
}

void* Zone::Segment::Allocate(size_t size) {
  auto const allocate_size = RoundUp(size, kAllocateUnit);
  auto const next_offset = offset_ + allocate_size;
  if (next_offset > max_offset_)
    return nullptr;
  auto const result = &memory_[offset_];
  offset_ = next_offset;
  return result;
}

//////////////////////////////////////////////////////////////////////
//
// Zone
//
Zone::Zone() : segment_(new Segment(0, nullptr)) {
}

Zone::~Zone() {
  auto segment = segment_;
  while (segment) {
    auto const next_segment = segment->next();
    delete segment;
    segment = next_segment;
  }
}

void* Zone::Allocate(size_t size) {
  for (;;) {
    if (auto const pointer = segment_->Allocate(size))
      return pointer;
    segment_ = new Segment(size, segment_);
  }
}

}  // namespace elang
