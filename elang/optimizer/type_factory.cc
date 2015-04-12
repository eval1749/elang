// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/type_factory.h"

#include <map>
#include <utility>

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/optimizer/factory_config.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {
typedef std::pair<Type*, std::vector<int>> ArrayProperty;
typedef std::pair<Type*, Type*> TypePair;

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
  std::map<ArrayProperty, ArrayType*> map_;
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
  std::map<TypePair, FunctionType*> map_;
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
  auto const new_type = new (zone_) FunctionType(return_type, parameters_type);
  map_[key] = new_type;
  return new_type;
}

//////////////////////////////////////////////////////////////////////
//
// TypeFactory::TupleTypeFactory
//
class TypeFactory::TupleTypeFactory final {
 public:
  explicit TupleTypeFactory(Zone* zone) : zone_(zone) {}
  ~TupleTypeFactory() = default;

  TupleType* NewTupleType(const std::vector<Type*>& members);

 private:
  std::map<std::vector<Type*>, TupleType*> map_;
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(TupleTypeFactory);
};

TupleType* TypeFactory::TupleTypeFactory::NewTupleType(
    const std::vector<Type*>& members) {
  const auto it = map_.find(members);
  if (it != map_.end())
    return it->second;
  auto const new_type = new (zone_) TupleType(zone_, members);
  map_[members] = new_type;
  return new_type;
}

//////////////////////////////////////////////////////////////////////
//
// TypeFactory
//
TypeFactory::TypeFactory(const FactoryConfig& config)
    : effect_type_(new (zone()) EffectType()),
      string_type_(new (zone()) StringType(config.string_type_name)),
#define V(Name, name, ...) name##_type_(new (zone()) Name##Type()),
      FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V
          array_type_factory_(new ArrayTypeFactory(zone())),
      function_type_factory_(new FunctionTypeFactory(zone())),
      tuple_type_factory_(new TupleTypeFactory(zone())),
      control_type_(NewControlType(void_type_)) {
}

TypeFactory::~TypeFactory() {
}

#define V(Name, name, ...) \
  Name##Type* TypeFactory::name##_type() const { return name##_type_; }
FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V

ArrayType* TypeFactory::NewArrayType(Type* element_type,
                                     const std::vector<int>& dimensions) {
#if _DEBUG
  for (auto const dimension : dimensions) {
    // dimension == -1 means unbound dimension.
    DCHECK_GE(dimension, -1);
  }
#endif
  return array_type_factory_->NewArrayType(element_type, dimensions);
}

ControlType* TypeFactory::NewControlType(Type* data_type) {
  auto it = control_type_map_.find(data_type);
  if (it != control_type_map_.end())
    return it->second;
  auto const new_control_type = new (zone()) ControlType(data_type);
  control_type_map_[data_type] = new_control_type;
  return new_control_type;
}

ExternalType* TypeFactory::NewExternalType(AtomicString* name) {
  return new (zone()) ExternalType(name);
}

FunctionType* TypeFactory::NewFunctionType(Type* return_type,
                                           Type* parameters_type) {
  return function_type_factory_->NewFunctionType(return_type, parameters_type);
}

PointerType* TypeFactory::NewPointerType(Type* pointee) {
  auto it = pointer_type_map_.find(pointee);
  if (it != pointer_type_map_.end())
    return it->second;
  auto const new_pointer_type = new (zone()) PointerType(pointee);
  pointer_type_map_[pointee] = new_pointer_type;
  return new_pointer_type;
}

TupleType* TypeFactory::NewTupleType(const std::vector<Type*>& components) {
  DCHECK_GE(components.size(), 2u);
  return tuple_type_factory_->NewTupleType(components);
}

}  // namespace optimizer
}  // namespace elang
