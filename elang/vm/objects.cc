// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/vm/objects.h"

#include "base/logging.h"
#include "elang/vm/factory.h"

namespace elang {
namespace vm {
namespace impl {

void* Object::operator new(size_t size, Factory* factory, Type* type) {
  DCHECK_EQ(size, type->instance_size);
  return factory->NewDataBlob(size);
}

}  // namespace impl
}  // namespace vm
}  // namespace elang
