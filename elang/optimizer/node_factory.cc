// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "elang/optimizer/node_factory.h"

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "elang/base/zone.h"
#include "elang/base/zone_user.h"
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
// NodeFactory::LiteralNodeCache
//
class NodeFactory::LiteralNodeCache final : public ZoneUser {
 public:
  class DefaultValueFactory;

  LiteralNodeCache(Zone* zone, TypeFactory* type_factory);
  ~LiteralNodeCache();

#define V(Name, mnemonic, data_type, ...) \
  Node* New##Name(Type* type, data_type data);
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
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
  std::unordered_map<base::StringPiece16, Node*> string_cache_;
  TypeFactory* const type_factory_;

  DISALLOW_COPY_AND_ASSIGN(LiteralNodeCache);
};

NodeFactory::LiteralNodeCache::LiteralNodeCache(Zone* zone,
                                                TypeFactory* type_factory)
    : ZoneUser(zone), type_factory_(type_factory) {
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

Node* NodeFactory::LiteralNodeCache::NewString(Type* type,
                                               base::StringPiece16 data) {
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

//////////////////////////////////////////////////////////////////////
//
// NodeFactory::LiteralNodeCache::DefaultValueFactory
//
class NodeFactory::LiteralNodeCache::DefaultValueFactory : public TypeVisitor {
 public:
  explicit DefaultValueFactory(LiteralNodeCache* cache);
  ~DefaultValueFactory() = default;

  Node* value() const {
    DCHECK(value_);
    return value_;
  }

 private:
  Node* value_;

  // TypeVisitor
  void DoDefaultVisit(Type* type) final;

#define V(Name, ...) void Visit##Name##Type(Name##Type* type) final;
  FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

  LiteralNodeCache* const cache_;

  DISALLOW_COPY_AND_ASSIGN(DefaultValueFactory);
};

NodeFactory::LiteralNodeCache::DefaultValueFactory::DefaultValueFactory(
    LiteralNodeCache* cache)
    : cache_(cache), value_(nullptr) {
}

void NodeFactory::LiteralNodeCache::DefaultValueFactory::DoDefaultVisit(
    Type* type) {
  DCHECK(!type->is<PrimitiveValueType>());
  value_ = cache_->NewNull(type);
}

#define V(Name, name, data_type, ...)                                         \
  void NodeFactory::LiteralNodeCache::DefaultValueFactory::Visit##Name##Type( \
      Name##Type* type) {                                                     \
    value_ = cache_->New##Name(type, static_cast<data_type>(0));              \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
NodeFactory::NodeFactory(TypeFactory* type_factory)
    : TypeFactoryUser(type_factory),
      literal_node_cache_(new LiteralNodeCache(zone(), type_factory)),
      node_id_source_(new SequenceIdSource()),
      false_value_(NewBool(false)),
      true_value_(NewBool(true)),
      void_value_(new (zone()) VoidNode(void_type())) {
}

NodeFactory::~NodeFactory() {
}

Node* NodeFactory::DefaultValueOf(Type* type) {
  if (type == void_type())
    return void_value_;
  LiteralNodeCache::DefaultValueFactory factory(literal_node_cache_.get());
  type->Accept(&factory);
  return factory.value();
}

Node* NodeFactory::NewFunctionReference(Function* function) {
  auto const output_type = NewPointerType(function->function_type());
  return literal_node_cache_->NewFunctionReference(output_type, function);
}

size_t NodeFactory::NewNodeId() {
  return node_id_source_->NextId();
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
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(effect->IsValidEffect()) << *effect;
  auto const node = new (zone()) ExitNode(void_type(), zone());
  node->AppendInput(control);
  node->AppendInput(effect);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewRet(Node* control, Node* value) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(value->IsValidValue()) << *value;
  auto const node = new (zone()) RetNode(control_type(), control, value);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewString(base::StringPiece16 data) {
  return literal_node_cache_->NewString(string_type(), data);
}

}  // namespace optimizer
}  // namespace elang
