// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/vm/factory.h"

#include "base/logging.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/base/zone.h"
#include "elang/vm/class.h"
#include "elang/vm/machine_code_collection.h"
#include "elang/vm/memory_pool.h"
#include "elang/vm/namespace.h"

namespace elang {
namespace vm {

namespace {

Namespace* CreateGlobalNamespace(Factory* factory) {
  return factory->NewNamespace(nullptr, factory->NewAtomicString(L"."));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory()
    : atomic_string_factory_(new AtomicStringFactory()),
      code_memory_pool_(new MemoryPool(MemoryPool::Kind::Code, 16)),
      data_memory_pool_(new MemoryPool(MemoryPool::Kind::Data, 16)),
      global_namespace_(CreateGlobalNamespace(this)),
      machine_code_collection_(new MachineCodeCollection(this)) {
}

Factory::~Factory() {
}

AtomicString* Factory::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

Class* Factory::NewClass(Namespace* outer,
                         AtomicString* name,
                         const std::vector<Class*>& base_classes) {
  return new (zone()) Class(zone(), outer, name, base_classes);
}

EntryPoint Factory::NewCodeBlob(size_t size) {
  return reinterpret_cast<EntryPoint>(
      code_memory_pool_->Allocate(static_cast<int>(size)));
}

void* Factory::NewDataBlob(size_t size) {
  return data_memory_pool_->Allocate(static_cast<int>(size));
}

Namespace* Factory::NewNamespace(Namespace* outer, AtomicString* name) {
  return new (zone()) Namespace(zone(), outer, name);
}

base::StringPiece16 Factory::NewString(base::StringPiece16 string) {
  return atomic_string_factory_->NewString(string);
}

}  // namespace vm
}  // namespace elang
