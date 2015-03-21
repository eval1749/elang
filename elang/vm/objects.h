// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_OBJECTS_H_
#define ELANG_VM_OBJECTS_H_

#include "base/basictypes.h"

namespace elang {
namespace vm {
namespace impl {

struct ClassDescription {
  uint32_t value;
};

struct Object {
  ClassDescription* class_description;
};

struct Vector final : Object {
  int32_t length;
  void* first_element;
};

struct String final : Object {
  Vector* characters;
};

}  // namespace impl
}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_OBJECTS_H_
