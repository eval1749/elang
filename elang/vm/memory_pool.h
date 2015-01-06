// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_MEMORY_POOL_H_
#define ELANG_VM_MEMORY_POOL_H_

#include "base/macros.h"
#include "elang/base/double_linked.h"

namespace elang {
namespace vm {

//////////////////////////////////////////////////////////////////////
//
// MemoryPool
//
class MemoryPool {
 public:
  class Segment;

  enum class Kind {
    Code,
    Data,
  };

  explicit MemoryPool(Kind kind, int alignment);

  void* Allocate(int size);

 private:
  int const alignment_;
  Kind const kind_;
  DoubleLinked<Segment, MemoryPool> large_blob_segment_;
  DoubleLinked<Segment, MemoryPool> small_blob_segment_;

  DISALLOW_COPY_AND_ASSIGN(MemoryPool);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_MEMORY_POOL_H_
