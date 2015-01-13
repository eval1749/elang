// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_PLATFORM_VIRTUAL_MEMORY_H_
#define ELANG_VM_PLATFORM_VIRTUAL_MEMORY_H_

#include "base/macros.h"
namespace elang {
namespace vm {

class VirtualMemory final {
 public:
  VirtualMemory(const VirtualMemory& other) = delete;
  explicit VirtualMemory(size_t size);
  VirtualMemory(VirtualMemory&& other);
  ~VirtualMemory();

  VirtualMemory& operator=(const VirtualMemory& other) = delete;
  VirtualMemory& operator=(VirtualMemory&& other);

  void* address() const { return address_; }
  size_t size() const { return size_; }

  void* CommitCode();
  void* CommitData();
  void* CommitGuard();

 private:
  void* address_;
  size_t size_;
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_PLATFORM_VIRTUAL_MEMORY_H_
