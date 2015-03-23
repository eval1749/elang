// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "elang/optimizer/node_factory.h"

#include "elang/base/zone.h"
#include "elang/base/zone_user.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// NodeFactory::LiteralNodeCache
//
class NodeFactory::LiteralNodeCache final : public ZoneUser {
 public:
  LiteralNodeCache(Zone* zone, TypeFactory* type_factory);
  ~LiteralNodeCache();

#define V(Name, name, data_type, ...) \
  Node* New##Name(Type* type, data_type data);
  FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V
  Node* NewFunctionReference(Type* output_type, Function* function);
  Node* NewNull(Type* type);

 private:
#define V(Name, name, data_type, ...) \
  std::unordered_map<data_type, Node*> name##_cache_;
  FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V
  std::unordered_map<Function*, Node*> function_literal_cache_;
  std::unordered_map<Type*, Node*> null_literal_cache_;
  TypeFactory* const type_factory_;

  DISALLOW_COPY_AND_ASSIGN(LiteralNodeCache);
};

NodeFactory::LiteralNodeCache::LiteralNodeCache(Zone* zone,
                                                TypeFactory* type_factory)
    : ZoneUser(zone), type_factory_(type_factory) {
#define V(Name, name, data_type, ...) \
  name##_cache_[0] = type_factory->name##_type()->default_value();
  FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V
}

NodeFactory::LiteralNodeCache::~LiteralNodeCache() {
}

#define V(Name, name, data_type, ...)                                          \
  Node* NodeFactory::LiteralNodeCache::New##Name(Type* type, data_type data) { \
    auto const it = name##_cache_.find(data);                                  \
    if (it != name##_cache_.end())                                             \
      return it->second;                                                       \
    auto const literal = new (zone()) Name##Node(type, data);                  \
    name##_cache_[data] = literal;                                             \
    return literal;                                                            \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

Node* NodeFactory::LiteralNodeCache::NewFunctionReference(Type* output_type,
                                                          Function* function) {
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

Node* NodeFactory::LiteralNodeCache::NewNull(Type* type) {
  auto const it = null_literal_cache_.find(type);
  if (it != null_literal_cache_.end())
    return it->second;
  auto const literal = new (zone()) NullNode(type);
  null_literal_cache_[type] = literal;
  return literal;
}

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
NodeFactory::NodeFactory(TypeFactory* type_factory)
    : TypeFactoryUser(type_factory),
      last_node_id_(0),
      literal_node_cache_(new LiteralNodeCache(zone(), type_factory)),
      false_value_(NewBool(false)),
      true_value_(NewBool(true)) {
}

NodeFactory::~NodeFactory() {
}

Function* NodeFactory::NewFunction(FunctionType* function_type) {
  auto const entry_node = NewEntry(function_type->parameters_type());
  auto const ret_node =
      NewRet(entry_node, function_type->return_type()->default_value());
  auto const exit_node = NewExit(ret_node, entry_node);
  auto const function =
      new (zone()) Function(function_type, entry_node, exit_node);
  return function;
}

Node* NodeFactory::NewFunctionReference(Function* function) {
  auto const output_type = NewPointerType(function->function_type());
  return literal_node_cache_->NewFunctionReference(output_type, function);
}

size_t NodeFactory::NewNodeId() {
  return ++last_node_id_;
}

Node* NodeFactory::NewNull(Type* type) {
  return literal_node_cache_->NewNull(type);
}

// Arithmetic nodes
#define V(Name, ...)                                                        \
  Node* NodeFactory::New##Name(Node* input0, Node* input1) {                \
    auto const output_type = input0->output_type();                         \
    DCHECK_EQ(output_type, input1->output_type());                          \
    auto const node = new (zone()) Name##Node(output_type, input0, input1); \
    node->set_id(NewNodeId());                                              \
    return node;                                                            \
  }
FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V)
#undef V

// Literal nodes
#define V(Name, name, data_type, ...)                           \
  Node* NodeFactory::New##Name(data_type data) {                \
    return literal_node_cache_->New##Name(name##_type(), data); \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

Node* NodeFactory::NewEntry(Type* parameters_type) {
  auto const output_type =
      NewTupleType({control_type(), effect_type(), parameters_type});
  auto const node = new (zone()) EntryNode(output_type);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewExit(Node* control, Node* effect) {
  DCHECK(control->IsControl()) << *control;
  DCHECK(effect->IsEffect()) << *effect;
  auto const node = new (zone()) ExitNode(void_type(), zone());
  node->AppendInput(control);
  node->AppendInput(effect);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewRet(Node* control, Node* value) {
  DCHECK(control->IsControl()) << *control;
  auto const node = new (zone()) RetNode(control_type(), control, value);
  node->set_id(NewNodeId());
  return node;
}

}  // namespace optimizer
}  // namespace elang
