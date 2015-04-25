// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/vm/memory_pool.h"

#include "elang/vm/platform/virtual_memory.h"

namespace elang {
namespace vm {

namespace {
const int kLargeDataThreshold = 1024 * 1;

size_t RoundUp(size_t num, size_t unit) {
  return ((num + unit - 1) / unit) * unit;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MemoryPool::Segment
//
class MemoryPool::Segment final
    : public DoubleLinked<Segment, MemoryPool>::NodeBase {
 public:
  Segment(Kind kind, size_t size);
  ~Segment() = default;

  void* Allocate(size_t size);

 private:
  VirtualMemory memory_;
  size_t offset_;
  size_t const size_;

  DISALLOW_COPY_AND_ASSIGN(Segment);
};

MemoryPool::Segment::Segment(Kind kind, size_t size)
    : memory_(VirtualMemory(size)), offset_(0), size_(memory_.size()) {
  if (kind == Kind::Code)
    memory_.CommitCode();
  else
    memory_.CommitData();
}

void* MemoryPool::Segment::Allocate(size_t size) {
  auto const new_offset = offset_ + size;
  // TODO(eval1749) We should remember rest of memory in segment.
  if (new_offset > size_)
    return nullptr;
  auto const result = static_cast<uint8_t*>(memory_.address()) + offset_;
  offset_ = new_offset;
  return result;
}

//////////////////////////////////////////////////////////////////////
//
// MemoryPool
//
MemoryPool::MemoryPool(Kind kind, size_t alignment)
    : alignment_(alignment), kind_(kind) {
  large_blob_segment_.AppendNode(new Segment(kind_, 1));
  small_blob_segment_.AppendNode(new Segment(kind_, 1));
}

void* MemoryPool::Allocate(size_t requested_size) {
  auto const size = RoundUp(requested_size, alignment_);
  if (size > kLargeDataThreshold) {
    for (;;) {
      if (auto const address = large_blob_segment_.last_node()->Allocate(size))
        return address;
      large_blob_segment_.AppendNode(new Segment(kind_, size));
    }
  }
  for (;;) {
    if (auto const address = small_blob_segment_.last_node()->Allocate(size))
      return address;
    small_blob_segment_.AppendNode(new Segment(kind_, size));
  }
}

}  // namespace vm
}  // namespace elang
