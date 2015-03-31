// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>
#include <unordered_map>
#include <utility>

#include "elang/optimizer/node_cache.h"

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "elang/base/zone.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/sequence_id_source.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"
#include "elang/optimizer/type_visitor.h"

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

Node* NodeCache::FindBinaryNode(Opcode opcode, Node* left, Node* right) {
  auto const it = binary_node_cache_.find(std::make_tuple(opcode, left, right));
  return it == binary_node_cache_.end() ? nullptr : it->second;
}

Node* NodeCache::FindFieldNode(Node* input, size_t field) {
  auto const it = field_node_cache_.find(std::make_tuple(input, field));
  return it == field_node_cache_.end() ? nullptr : it->second;
}

Node* NodeCache::FindUnaryNode(Opcode opcode, Type* type, Node* input) {
  auto const it = unary_node_cache_.find(std::make_tuple(opcode, type, input));
  return it == unary_node_cache_.end() ? nullptr : it->second;
}

#define V(Name, name, data_type, ...)                         \
  Data* NodeCache::New##Name(Type* type, data_type data) {    \
    auto const it = name##_cache_.find(data);                 \
    if (it != name##_cache_.end())                            \
      return it->second;                                      \
    auto const literal = new (zone()) Name##Node(type, data); \
    name##_cache_[data] = literal;                            \
    return literal;                                           \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

Data* NodeCache::NewFunctionReference(Type* output_type, Function* function) {
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

size_t NodeCache::NewNodeId() {
  return node_id_source_->NextId();
}

Data* NodeCache::NewNull(Type* type) {
  auto const it = null_literal_cache_.find(type);
  if (it != null_literal_cache_.end())
    return it->second;
  auto const literal = new (zone()) NullNode(type);
  null_literal_cache_[type] = literal;
  return literal;
}

Data* NodeCache::NewReference(Type* type, AtomicString* name) {
  auto const key = std::make_tuple(type, name);
  auto const it = reference_cache_.find(key);
  if (it != reference_cache_.end())
    return it->second;
  auto const new_node = new (zone()) ReferenceNode(type, name);
  reference_cache_[key] = new_node;
  return new_node;
}

Data* NodeCache::NewString(Type* type, base::StringPiece16 data) {
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

void NodeCache::RememberBinaryNode(Node* node) {
  auto const key =
      std::make_tuple(node->opcode(), node->input(0), node->input(1));
  DCHECK(!binary_node_cache_.count(key));
  binary_node_cache_[key] = node;
}

void NodeCache::RememberFieldNode(Node* node, Node* input, size_t field) {
  auto const key = std::make_tuple(input, field);
  DCHECK(!field_node_cache_.count(key));
  field_node_cache_[key] = node;
}

void NodeCache::RememberUnaryNode(Node* node) {
  auto const key =
      std::make_tuple(node->opcode(), node->output_type(), node->input(0));
  DCHECK(!unary_node_cache_.count(key));
  unary_node_cache_[key] = node;
}

}  // namespace optimizer
}  // namespace elang
