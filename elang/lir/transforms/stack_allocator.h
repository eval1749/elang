// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_STACK_ALLOCATOR_H_
#define ELANG_LIR_TRANSFORMS_STACK_ALLOCATOR_H_

#include <vector>

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

struct Value;

//////////////////////////////////////////////////////////////////////
//
// StackAllocator
//
class ELANG_LIR_EXPORT StackAllocator final {
 public:
  explicit StackAllocator(int alignment);
  ~StackAllocator();

  int current_size() const { return static_cast<int>(uses_.size()); }
  int maximum_argc() const { return maximum_argc_; }

  void AllocateAt(Value stack_slot);
  Value Allocate(Value type);
  void Free(Value location);
  int RequiredSize() const;
  void Reset();
  void TrackNumberOfArguments(int number_of_arguments);

 private:
  int Allocate(int size);

  int const alignment_;
  int maximum_argc_;
  int maximum_size_;
  std::vector<bool> uses_;

  DISALLOW_COPY_AND_ASSIGN(StackAllocator);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_STACK_ALLOCATOR_H_
