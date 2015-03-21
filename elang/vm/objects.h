// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_OBJECTS_H_
#define ELANG_VM_OBJECTS_H_

#include "base/basictypes.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "elang/base/float_types.h"

namespace elang {
namespace vm {
class Factory;
namespace impl {

struct Type;

struct Object {
  void* operator new(size_t size, Factory* factory, Type* type);

  Type* type;
};

struct Type : Object {
  uint32_t instance_size;
  uint32_t value_size;
};

struct ArrayType final : Type {
  Type* element_type;
  int rank;
};

struct Class final : Type {};

struct VectorBase : Object {
  int32_t length;
};

static_assert(sizeof(VectorBase) == sizeof(void*) * 2,
              "sizeof(VectorBase) == sizeof(void*) * 2");

template <typename T>
struct Vector final : VectorBase {
  T* elements() { return reinterpret_cast<T*>(this + 1); }

  T& operator[](size_t index) {
    DCHECK_LT(index, static_cast<size_t>(length));
    return *(elements() + index);
  }
};

struct String final : Object {
  Vector<base::char16>* data;
};

struct ValueType : Object {};

struct Char final : ValueType {
  base::char16 data;
};

struct Float32 final : ValueType {
  float32_t data;
};

struct Float64 final : ValueType {
  float64_t data;
};

struct Int16 final : ValueType {
  int16_t data;
};

struct Int32 final : ValueType {
  int32_t data;
};

struct Int64 final : ValueType {
  int64_t data;
};

struct Int8 final : ValueType {
  int8_t data;
};

struct UInt16 final : ValueType {
  uint16_t data;
};

struct UInt32 final : ValueType {
  uint32_t data;
};

struct UInt64 final : ValueType {
  uint64_t data;
};

struct UInt8 final : ValueType {
  uint8_t data;
};

}  // namespace impl
}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_OBJECTS_H_
