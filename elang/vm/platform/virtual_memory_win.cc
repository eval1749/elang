// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/vm/platform/virtual_memory.h"

#include <windows.h>

#include "elang/base/logging_win.h"

namespace elang {
namespace vm {

namespace {
const size_t kAllocateUnit = 64 * 1024;

size_t RoundUp(size_t num, size_t unit) {
  return ((num + unit - 1) / unit) * unit;
}

void* Commit(void* address, size_t size, uint32_t protection) {
  VERIFY_WIN32API(::VirtualAlloc(address, size, MEM_COMMIT, protection));
  return address;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// VirtualMemory
//
VirtualMemory::VirtualMemory(size_t size) {
  DCHECK(size);
  size_ = RoundUp(size, kAllocateUnit);
  address_ = ::VirtualAlloc(nullptr, size_, MEM_RESERVE, PAGE_NOACCESS);
  VERIFY_WIN32API(address_);
}

VirtualMemory::VirtualMemory(VirtualMemory&& other)
    : address_(other.address_), size_(other.size_) {
  other.address_ = nullptr;
  other.size_ = 0u;
}

VirtualMemory::~VirtualMemory() {
  if (!address_)
    return;
  VERIFY_WIN32API(::VirtualFree(address_, size_, MEM_RELEASE));
}

void* VirtualMemory::CommitCode() {
  return Commit(address_, size_, PAGE_EXECUTE_READWRITE);
}

void* VirtualMemory::CommitData() {
  return Commit(address_, size_, PAGE_READWRITE);
}

void* VirtualMemory::CommitGuard() {
  return Commit(address_, size_, PAGE_GUARD);
}

}  // namespace vm
}  // namespace elang
