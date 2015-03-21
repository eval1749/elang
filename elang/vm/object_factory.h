// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_OBJECT_FACTORY_H_
#define ELANG_VM_OBJECT_FACTORY_H_

#include <array>
#include <unordered_map>
#include <string>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"

namespace elang {
namespace vm {
class Factory;
namespace impl {
struct ArrayType;
struct Class;
struct Object;
struct String;
struct Type;
template <typename T>
struct Vector;
struct VectorBase;

//////////////////////////////////////////////////////////////////////
//
// ObjectFactory
//
class ObjectFactory final {
 public:
  ~ObjectFactory();

  Class* char_class() const { return char_class_; }
  Class* class_meta_class() const { return class_meta_class_; }
  Factory* factory() const { return factory_; }
  Class* string_class() const { return string_class_; }

  String* NewString(base::StringPiece16 data);

  template <typename T>
  Vector<T>* NewVector(Type* element_type, size_t size) {
    return static_cast<Vector<T>*>(NewVectorBase(element_type, size));
  }

 private:
  friend class Factory;

  explicit ObjectFactory(Factory* factory);

  ArrayType* NewArrayType(Type* element_type, int rank);
  VectorBase* NewVectorBase(Type* element_type, size_t size);

  Factory* const factory_;
  std::array<std::unordered_map<Type*, ArrayType*>, 7> array_types_;

  Class* const object_class_;
  Class* const class_meta_class_;
  Class* const array_meta_class_;
  Class* const char_class_;
  ArrayType* const char_vector_type_;
  Class* const string_class_;

  DISALLOW_COPY_AND_ASSIGN(ObjectFactory);
};

}  // namespace impl
}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_OBJECT_FACTORY_H_
