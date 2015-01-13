// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/vm/collectable.h"

#include "base/logging.h"
#include "elang/vm/factory.h"

namespace elang {
namespace vm {

void Collectable::operator delete(void* pointer, Factory* factory) {
  __assume(pointer);
  __assume(factory);
  NOTREACHED();
}

void* Collectable::operator new(size_t size, Factory* factory) {
  return factory->NewDataBlob(static_cast<int>(size));
}

Collectable::Collectable() {
}

Collectable::~Collectable() {
  NOTREACHED();
}

}  // namespace vm
}  // namespace elang
