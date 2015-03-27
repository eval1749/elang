// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "elang/optimizer/node_cache.h"

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "elang/base/index_sequence.h"
#include "elang/base/zone.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"
#include "elang/optimizer/type_visitor.h"

namespace elang {
namespace optimizer {

template <typename... Keys>
struct KeyTuple {
  std::tuple<Keys...> components;

  KeyTuple(const Keys&... components)
      : components(std::make_tuple(components...)) {}

  bool operator<(const KeyTuple& other) const {
    return less(*this, other, MakeIndexSequence<sizeof...(Keys)>());
  }

  template <size_t K>
  static bool less(const KeyTuple& lhs, const KeyTuple& rhs, IndexSequence<K>) {
    auto& lhs_component = std::get<K>(lhs.components);
    auto& rhs_component = std::get<K>(rhs.components);
    return lhs_component < rhs_component;
  }

  template <size_t K, size_t... Ks>
  static bool less(const KeyTuple& lhs,
                   const KeyTuple& rhs,
                   IndexSequence<K, Ks...>) {
    auto& lhs_component = std::get<K>(lhs.components);
    auto& rhs_component = std::get<K>(rhs.components);
    if (lhs_component != rhs_component)
      return lhs_component < rhs_component;
    return less(lhs, rhs, IndexSequence<Ks...>());
  }
};

template <typename... Keys>
KeyTuple<Keys...> make_key_tuple(const Keys&... params...) {
  return KeyTuple<Keys...>(params...);
}

}  // namespace optimizer
}  // namespace elang

namespace std {
template <typename... Keys>
struct less<::elang::optimizer::KeyTuple<Keys...>> {
  typedef ::elang::optimizer::KeyTuple<Keys...> KeyTuple;
  bool operator()(const KeyTuple& lhs, const KeyTuple& rhs) {
    return lhs < rhs;
  }
};
}  // namespace std

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// NodeCache
//
NodeCache::NodeCache(Zone* zone, TypeFactory* type_factory)
    : ZoneUser(zone),
      node_id_source_(new SequenceIdSource()),
      type_factory_(type_factory) {
}

NodeCache::~NodeCache() {
}

#define V(Name, name, data_type, ...)                         \
  Node* NodeCache::New##Name(Type* type, data_type data) {    \
    auto const it = name##_cache_.find(data);                 \
    if (it != name##_cache_.end())                            \
      return it->second;                                      \
    auto const literal = new (zone()) Name##Node(type, data); \
    name##_cache_[data] = literal;                            \
    return literal;                                           \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

Node* NodeCache::NewFunctionReference(Type* output_type, Function* function) {
  DCHECK_EQ(output_type->as<PointerType>()->pointee(),
            function->function_type());
  auto const it = function_literal_cache_.find(function);
  if (it != function_literal_cache_.end())
    return it->second;
  auto const literal =
      new (zone()) FunctionReferenceNode(output_type, function);
  function_literal_cache_[function] = literal;
  return literal;
}

Node* NodeCache::NewGet(Node* input, size_t field) {
  auto const key = make_key_tuple(input, field);
  auto const it = field_input_node_cache_.find(key);
  if (it != field_input_node_cache_.end())
    return it->second;
  auto const output_type = input->output_type()->as<TupleType>()->get(field);
  auto const node = new (zone()) GetNode(output_type, input, field);
  node->set_id(NewNodeId());
  field_input_node_cache_[key] = node;
  return node;
}

size_t NodeFactory::NewNodeId() {
  return node_id_source_->NextId();
}

Node* NodeCache::NewNull(Type* type) {
  auto const it = null_literal_cache_.find(type);
  if (it != null_literal_cache_.end())
    return it->second;
  auto const literal = new (zone()) NullNode(type);
  null_literal_cache_[type] = literal;
  return literal;
}

Node* NodeCache::NewReference(Type* type, AtomicString* name) {
  auto const key = make_key_tuple(type, name);
  auto const it = reference_cache_.find(key);
  if (it != reference_cache_.end())
    return it->second;
  auto const new_node = new (zone()) ReferenceNode(type, name);
  reference_cache_[key] = new_node;
  return new_node;
}

Node* NodeCache::NewString(Type* type, base::StringPiece16 data) {
  auto const it = string_cache_.find(data);
  if (it != string_cache_.end())
    return it->second;

  auto const size = data.size() * sizeof(base::char16);
  auto const chars = static_cast<base::char16*>(zone()->Allocate(size));
  ::memcpy(chars, data.data(), size);
  base::StringPiece16 saved_data(chars, data.size());
  auto const literal = new (zone()) StringNode(type, saved_data);
  string_cache_[saved_data] = literal;
  return literal;
}

}  // namespace optimizer
}  // namespace elang
