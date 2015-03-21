// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/vm/object_factory.h"

#include "base/numerics/safe_conversions.h"
#include "base/strings/string16.h"
#include "elang/vm/factory.h"
#include "elang/vm/objects.h"

namespace elang {
namespace vm {
namespace impl {

namespace {
Class* NewObjectClass(Factory* factory) {
  auto const class_meta_class =
      reinterpret_cast<Class*>(factory->NewDataBlob(sizeof(Class)));
  class_meta_class->type = class_meta_class;
  class_meta_class->instance_size = base::checked_cast<uint32_t>(sizeof(Class));
  class_meta_class->value_size = base::checked_cast<uint32_t>(sizeof(Object*));

  auto const object_class = new (factory, class_meta_class) Class();
  object_class->type = class_meta_class;
  object_class->instance_size = 0;
  object_class->value_size = base::checked_cast<uint32_t>(sizeof(Object*));
  return object_class;
}

Class* NewClass(ObjectFactory* factory,
                size_t instance_size,
                size_t value_size) {
  auto const type =
      new (factory->factory(), factory->class_meta_class()) Class();
  type->type = factory->class_meta_class();
  type->instance_size = base::checked_cast<uint32_t>(instance_size);
  type->value_size = base::checked_cast<uint32_t>(value_size);
  return type;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ObjectFactory
//
ObjectFactory::ObjectFactory(Factory* factory)
    : factory_(factory),
      object_class_(NewObjectClass(factory)),
      class_meta_class_(reinterpret_cast<Class*>(object_class_->type)),
      array_meta_class_(NewClass(this, sizeof(ArrayType), sizeof(Object*))),
      char_class_(NewClass(this, sizeof(Char), sizeof(base::char16))),
      char_vector_type_(NewArrayType(char_class_, 1)),
      string_class_(NewClass(this, sizeof(String), sizeof(Object*))) {
}

ObjectFactory::~ObjectFactory() {
}

ArrayType* ObjectFactory::NewArrayType(Type* element_type, int rank) {
  auto& map = array_types_[rank];
  auto const it = map.find(element_type);
  if (it != map.end())
    return it->second;
  auto const type = new (factory_, array_meta_class_) ArrayType();
  type->instance_size = 0;
  type->value_size = base::checked_cast<uint32_t>(sizeof(Object*));
  type->element_type = element_type;
  type->rank = rank;
  map[element_type] = type;
  return type;
}

String* ObjectFactory::NewString(base::StringPiece16 data) {
  auto const chars = NewVector<base::char16>(char_class_, data.size());
  auto const string = new (factory_, string_class_) String();
  string->type = string_class_;
  string->data = chars;
  ::memcpy(chars->elements(), data.data(), sizeof(*data.data()) * data.size());
  return string;
}

VectorBase* ObjectFactory::NewVectorBase(Type* element_type, size_t length) {
  auto const element_size = element_type->value_size;
  auto const size = sizeof(VectorBase) +
                    std::max(length * element_size, sizeof(Object*)) -
                    sizeof(Object*);
  auto const vector =
      reinterpret_cast<VectorBase*>(factory_->NewDataBlob(size));
  vector->type = NewArrayType(element_type, 1);
  vector->length = base::checked_cast<int32_t>(length);
  return vector;
}

}  // namespace impl
}  // namespace vm
}  // namespace elang
