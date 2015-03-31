// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_NODE_CACHE_H_
#define ELANG_OPTIMIZER_NODE_CACHE_H_

#include <map>
#include <memory>
#include <tuple>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_user.h"
#include "elang/optimizer/nodes_forward.h"
#include "elang/optimizer/types_forward.h"

namespace elang {
class AtomicString;
class Zone;
namespace optimizer {

class SequenceIdSource;

//////////////////////////////////////////////////////////////////////
//
// NodeCache
//
class NodeCache final : public ZoneUser {
 public:
  NodeCache(Zone* zone, TypeFactory* type_factory);
  ~NodeCache();

  SequenceIdSource* node_id_source() const { return node_id_source_.get(); }

  size_t NewNodeId();

  // Cached nodes
  Node* FindBinaryNode(Opcode opcode, Node* left, Node* right);
  Node* FindProjectionNode(Node* input, size_t field);
  Node* FindUnaryNode(Opcode opcode, Type* type, Node* input);
  void RememberBinaryNode(Node* node);
  void RememberProjectionNode(Node* node, Node* input, size_t field);
  void RememberUnaryNode(Node* node);

// Cached node construct functions.
#define V(Name, mnemonic, data_type, ...) \
  Data* New##Name(Type* type, data_type data);
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V
  Data* NewFunctionReference(Type* output_type, Function* function);
  Node* NewGet(Node* input, size_t field);
  Data* NewNull(Type* type);
  Data* NewReference(Type* type, AtomicString* name);

 private:
#define V(Name, name, data_type, ...) \
  std::unordered_map<data_type, Data*> name##_cache_;
  FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V
  std::map<std::tuple<Opcode, Node*, Node*>, Node*> binary_node_cache_;
  std::unordered_map<Function*, Data*> function_literal_cache_;
  std::unordered_map<Type*, Data*> null_literal_cache_;
  const std::unique_ptr<SequenceIdSource> node_id_source_;
  std::map<std::tuple<Node*, size_t>, Node*> projection_node_cache_;
  std::map<std::tuple<Type*, AtomicString*>, Data*> reference_cache_;
  std::unordered_map<base::StringPiece16, Data*> string_cache_;
  std::map<std::tuple<Opcode, Type*, Node*>, Node*> unary_node_cache_;
  TypeFactory* const type_factory_;

  DISALLOW_COPY_AND_ASSIGN(NodeCache);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODE_CACHE_H_
