// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/type_factory.h"

#include <unordered_map>
#include <utility>

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/factory_config.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {
typedef std::pair<Type*, std::vector<int>> ArrayProperty;
typedef std::pair<Type*, Type*> TypePair;
}  // namespace hir
}  // namespace elang

namespace std {
using elang::hir::ArrayProperty;
using elang::hir::Type;
using elang::hir::TypePair;

template <>
struct hash<ArrayProperty> {
  size_t operator()(const ArrayProperty& pair) const {
    auto hash_code = std::hash<Type*>()(pair.first);
    for (auto const dimension : pair.second) {
      auto const upper = hash_code >> (sizeof(hash_code) - 3);
      hash_code <<= 3;
      hash_code ^= std::hash<int>()(dimension);
      hash_code ^= upper;
    }
    return hash_code;
  }
};

template <>
struct hash<TypePair> {
  size_t operator()(const TypePair& pair) const {
    return std::hash<Type*>()(pair.first) ^ std::hash<Type*>()(pair.second);
  }
};
}  // namespace std

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// TypeFactory::ArrayTypeFactory
//
class TypeFactory::ArrayTypeFactory final {
 public:
  explicit ArrayTypeFactory(Zone* zone) : zone_(zone) {}
  ~ArrayTypeFactory() = default;

  ArrayType* NewArrayType(Type* element_type,
                          const std::vector<int>& dimensions);

 private:
  std::unordered_map<ArrayProperty, ArrayType*> map_;
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(ArrayTypeFactory);
};

ArrayType* TypeFactory::ArrayTypeFactory::NewArrayType(
    Type* element_type,
    const std::vector<int>& dimensions) {
  const auto key = std::make_pair(element_type, dimensions);
  const auto it = map_.find(key);
  if (it != map_.end())
    return it->second;
  auto const new_type = new (zone_) ArrayType(zone_, element_type, dimensions);
  map_[key] = new_type;
  return new_type;
}

//////////////////////////////////////////////////////////////////////
//
// TypeFactory::FunctionTypeFactory
//
class TypeFactory::FunctionTypeFactory final {
 public:
  explicit FunctionTypeFactory(Zone* zone) : zone_(zone) {}
  ~FunctionTypeFactory() = default;

  FunctionType* NewFunctionType(Type* return_type, Type* parameters_type);

 private:
  std::unordered_map<TypePair, FunctionType*> map_;
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(FunctionTypeFactory);
};

FunctionType* TypeFactory::FunctionTypeFactory::NewFunctionType(
    Type* return_type,
    Type* parameters_type) {
  const auto key = std::make_pair(return_type, parameters_type);
  const auto it = map_.find(key);
  if (it != map_.end())
    return it->second;
  auto const new_type =
      new (zone_) FunctionType(zone_, return_type, parameters_type);
  map_[key] = new_type;
  return new_type;
}

//////////////////////////////////////////////////////////////////////
//
// TypeFactory
//
TypeFactory::TypeFactory(const FactoryConfig& config)
    :
#define V(Name, name, ...) name##_type_(new (zone()) Name##Type(zone())),
      FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
          array_type_factory_(new ArrayTypeFactory(zone())),
      function_type_factory_(new FunctionTypeFactory(zone())),
      string_type_(new (zone()) StringType(zone(), config.string_type_name)) {
}

TypeFactory::~TypeFactory() {
}

#define V(Name, name, ...) \
  Name##Type* TypeFactory::name##_type() const { return name##_type_; }
FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

ArrayType* TypeFactory::NewArrayType(Type* element_type,
                                     const std::vector<int>& dimensions) {
#if _DEBUG
  for (auto const dimension : dimensions) {
    DCHECK_GE(dimension, 0);
  }
#endif
  return array_type_factory_->NewArrayType(element_type, dimensions);
}

ExternalType* TypeFactory::NewExternalType(AtomicString* name) {
  return new (zone()) ExternalType(zone(), name);
}

FunctionType* TypeFactory::NewFunctionType(Type* return_type,
                                           Type* parameters_type) {
  return function_type_factory_->NewFunctionType(return_type, parameters_type);
}

PointerType* TypeFactory::NewPointerType(Type* pointee) {
  auto it = pointer_type_map_.find(pointee);
  if (it != pointer_type_map_.end())
    return it->second;
  auto const new_pointer_type = new (zone()) PointerType(zone(), pointee);
  pointer_type_map_[pointee] = new_pointer_type;
  return new_pointer_type;
}

TupleType* TypeFactory::NewTupleType(const std::vector<Type*>& members) {
  DCHECK_GE(members.size(), 2u);
#if _DEBUG
  for (auto const member : members) {
    DCHECK(!member->is<VoidType>());
  }
#endif
  return new (zone()) TupleType(zone(), members);
}

}  // namespace hir
}  // namespace elang
