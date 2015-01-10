// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/type_factory.h"

#include <unordered_map>
#include <utility>

#include "elang/base/zone.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {
typedef std::pair<Type*, Type*> TypePair;
}  // namespace hir
}  // namespace elang

namespace std {
using elang::hir::Type;
using elang::hir::TypePair;

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
TypeFactory::TypeFactory()
    :
#define V(Name, name, ...) name##_type_(new (zone()) Name##Type(zone())),
      FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
          function_type_factory_(new FunctionTypeFactory(zone())),
      string_type_(new (zone()) StringType(zone())) {
}

TypeFactory::~TypeFactory() {
}

#define V(Name, name, ...) \
  Name##Type* TypeFactory::Get##Name##Type() const { return name##_type_; }
FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

FunctionType* TypeFactory::NewFunctionType(Type* return_type,
                                           Type* parameters_type) {
  return function_type_factory_->NewFunctionType(return_type, parameters_type);
}

}  // namespace hir
}  // namespace elang
