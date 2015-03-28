// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "elang/optimizer/node_factory.h"

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_cache.h"
#include "elang/optimizer/sequence_id_source.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"
#include "elang/optimizer/type_visitor.h"

namespace elang {
namespace optimizer {

namespace {
//////////////////////////////////////////////////////////////////////
//
// DefaultValueFactory
//
class DefaultValueFactory : public TypeVisitor {
 public:
  explicit DefaultValueFactory(NodeCache* cache);
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

  NodeCache* const cache_;

  DISALLOW_COPY_AND_ASSIGN(DefaultValueFactory);
};

DefaultValueFactory::DefaultValueFactory(NodeCache* cache)
    : cache_(cache), value_(nullptr) {
}

void DefaultValueFactory::DoDefaultVisit(Type* type) {
  DCHECK(!type->is<PrimitiveValueType>());
  value_ = cache_->NewNull(type);
}

#define V(Name, name, data_type, ...)                             \
  void DefaultValueFactory::Visit##Name##Type(Name##Type* type) { \
    value_ = cache_->New##Name(type, static_cast<data_type>(0));  \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
NodeFactory::NodeFactory(TypeFactory* type_factory)
    : TypeFactoryUser(type_factory),
      node_cache_(new NodeCache(zone(), type_factory)),
      false_value_(NewBool(false)),
      true_value_(NewBool(true)),
      void_value_(new (zone()) VoidNode(void_type())) {
}

NodeFactory::~NodeFactory() {
}

SequenceIdSource* NodeFactory::node_id_source() const {
  return node_cache_->node_id_source();
}

Node* NodeFactory::DefaultValueOf(Type* type) {
  if (type == void_type())
    return void_value_;
  DefaultValueFactory factory(node_cache_.get());
  type->Accept(&factory);
  return factory.value();
}

Node* NodeFactory::NewCall(Node* effect, Node* callee, Node* arguments) {
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK(callee->IsValidData()) << *callee;
  DCHECK(callee->output_type()->is<FunctionType>()) << *callee;
  DCHECK(arguments->IsValidData()) << *arguments;
  auto const output_type =
      NewTupleType({effect_type(),
                    callee->output_type()->as<FunctionType>()->return_type()});
  auto const node =
      new (zone()) CallNode(output_type, effect, callee, arguments);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewFunctionReference(Function* function) {
  auto const output_type = NewPointerType(function->function_type());
  return node_cache_->NewFunctionReference(output_type, function);
}

size_t NodeFactory::NewNodeId() {
  return node_cache_->NewNodeId();
}

Node* NodeFactory::NewNull(Type* type) {
  return node_cache_->NewNull(type);
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
#define V(Name, name, data_type, ...)                   \
  Node* NodeFactory::New##Name(data_type data) {        \
    return node_cache_->New##Name(name##_type(), data); \
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

Node* NodeFactory::NewExit(Node* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) ExitNode(void_type(), control);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewGet(Node* input, size_t field) {
  DCHECK(input->IsValidData()) << *input;
  return node_cache_->NewGet(input, field);
}

Node* NodeFactory::NewIf(Node* control, Node* data) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(data->IsValidData()) << *data;
  auto const node = new (zone()) IfNode(control_type(), control, data);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewIfFalse(Node* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) IfFalseNode(control_type(), control);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewIfTrue(Node* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) IfTrueNode(control_type(), control);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewJump(Node* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) JumpNode(control_type(), control);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewMerge(const std::vector<Node*>& controls) {
  auto const node = new (zone()) MergeNode(control_type(), zone());
  node->set_id(NewNodeId());
  for (auto const control : controls) {
    DCHECK(control->IsValidControl()) << *control;
    node->AppendInput(control);
  }
  return node;
}

Node* NodeFactory::NewParameter(Node* input, size_t field) {
  auto const entry_node = input->as<EntryNode>();
  DCHECK(entry_node) << *input;
  auto const output_type = entry_node->parameter_type(field);
  auto const node = new (zone()) ParameterNode(output_type, input, field);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewPhi(Type* output_type, Node* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const owner = control->as<PhiOwnerNode>();
  DCHECK(owner) << *control;
  auto const node = new (zone()) PhiNode(output_type, zone(), owner);
  node->set_id(NewNodeId());
  owner->phi_nodes_.AppendNode(node);
  return node;
}

Node* NodeFactory::NewPhiOperand(Node* control, Node* data) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(data->IsValidData()) << *data;
  auto const output_type = NewTupleType({control_type(), data->output_type()});
  auto const node = new (zone()) PhiOperandNode(output_type, control, data);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewReference(Type* type, AtomicString* name) {
  return node_cache_->NewReference(type, name);
}

Node* NodeFactory::NewRet(Node* control, Node* effect, Node* data) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(data->IsValidData()) << *data;
  auto const node = new (zone()) RetNode(control_type(), control, effect, data);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewString(base::StringPiece16 data) {
  return node_cache_->NewString(string_type(), data);
}

Node* NodeFactory::NewTuple(const std::vector<Node*>& inputs) {
  std::vector<Type*> types(inputs.size());
  types.resize(0);
  for (auto input : inputs)
    types.push_back(input->output_type());
  auto const output_type = NewTupleType(types);
  auto const node = new (zone()) TupleNode(output_type, zone());
  for (auto input : inputs) {
    DCHECK(input->IsValidData()) << *input;
    node->AppendInput(input);
  }
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewTuple(Type* output_type) {
  auto const node = new (zone()) TupleNode(output_type, zone());
  node->set_id(NewNodeId());
  return node;
}

}  // namespace optimizer
}  // namespace elang
