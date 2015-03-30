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

Node* NodeFactory::FindBinaryNode(Opcode opcode, Node* left, Node* right) {
  return node_cache_->FindBinaryNode(opcode, left, right);
}

Node* NodeFactory::FindFieldNode(Node* input, size_t field) {
  return node_cache_->FindFieldNode(input, field);
}

Node* NodeFactory::FindUnaryNode(Opcode opcode, Type* type, Node* input) {
  return node_cache_->FindUnaryNode(opcode, type, input);
}

Node* NodeFactory::NewCall(Effect* effect, Node* callee, Node* arguments) {
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

Control* NodeFactory::NewControlGet(Node* input, size_t field) {
  DCHECK(input->IsValidControlAt(field)) << *input;
  if (auto const present = FindFieldNode(input, field)->as<ControlGetNode>())
    return present;
  auto const node = new (zone()) ControlGetNode(control_type(), input, field);
  node->set_id(NewNodeId());
  RememberFieldNode(node, input, field);
  return node;
}

Node* NodeFactory::NewDynamicCast(Type* type, Node* input) {
  if (input->output_type() == type)
    return input;
  if (auto const present = FindUnaryNode(Opcode::DynamicCast, type, input))
    return present;
  auto const node = new (zone()) DynamicCastNode(type, input);
  node->set_id(NewNodeId());
  return node;
}

Effect* NodeFactory::NewEffectGet(Node* input, size_t field) {
  DCHECK(input->IsValidEffectAt(field)) << *input;
  if (auto const present = FindFieldNode(input, field)->as<EffectGetNode>())
    return present;
  auto const node = new (zone()) EffectGetNode(effect_type(), input, field);
  node->set_id(NewNodeId());
  RememberFieldNode(node, input, field);
  return node;
}

EffectPhiNode* NodeFactory::NewEffectPhi(PhiOwnerNode* owner) {
  DCHECK(owner->IsValidControl()) << *owner;
  auto const node = new (zone()) EffectPhiNode(effect_type(), zone(), owner);
  node->set_id(NewNodeId());
  owner->set_effect_phi(node);
  return node;
}

Node* NodeFactory::NewEntry(Type* parameters_type) {
  auto const output_type =
      NewTupleType({control_type(), effect_type(), parameters_type});
  auto const node = new (zone()) EntryNode(output_type);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewExit(Control* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) ExitNode(void_type(), control);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewFloatAdd(Node* left, Node* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewFloatAdd(right, left);
  if (auto const present = FindBinaryNode(Opcode::FloatAdd, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatAddNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewFloatCmp(FloatCondition condition,
                               Node* left,
                               Node* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node =
      new (zone()) FloatCmpNode(bool_type(), condition, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewFloatDiv(Node* left, Node* right) {
  if (auto const present = FindBinaryNode(Opcode::FloatDiv, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatDivNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewFloatMul(Node* left, Node* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewFloatAdd(right, left);
  if (auto const present = FindBinaryNode(Opcode::FloatMul, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatMulNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewFloatMod(Node* left, Node* right) {
  if (auto const present = FindBinaryNode(Opcode::FloatMod, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatModNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewFloatSub(Node* left, Node* right) {
  if (auto const present = FindBinaryNode(Opcode::FloatSub, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatSubNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewFunctionReference(Function* function) {
  auto const output_type = NewPointerType(function->function_type());
  return node_cache_->NewFunctionReference(output_type, function);
}

Node* NodeFactory::NewGet(Node* input, size_t field) {
  DCHECK(input->id() || input->IsLiteral()) << *input << " " << field;
  if (auto const present = FindFieldNode(input, field))
    return present;
  auto const output_type = input->output_type()->as<TupleType>()->get(field);
  auto const node = new (zone()) GetNode(output_type, input, field);
  node->set_id(NewNodeId());
  RememberFieldNode(node, input, field);
  return node;
}

Control* NodeFactory::NewIf(Control* control, Node* data) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(data->IsValidData()) << *data;
  auto const node = new (zone()) IfNode(control_type(), control, data);
  node->set_id(NewNodeId());
  return node;
}

Control* NodeFactory::NewIfFalse(Control* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) IfFalseNode(control_type(), control);
  node->set_id(NewNodeId());
  return node;
}

Control* NodeFactory::NewIfTrue(Control* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) IfTrueNode(control_type(), control);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewIntAdd(Node* left, Node* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntAdd(right, left);
  if (auto const present = FindBinaryNode(Opcode::IntAdd, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(0) || right == NewInt64(0))
    return left;
  auto const node = new (zone()) IntAddNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntBitAnd(Node* left, Node* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntBitAnd(right, left);
  if (auto const present = FindBinaryNode(Opcode::IntBitAnd, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(0))
    return NewInt32(0);
  if (right == NewInt64(0))
    return NewInt64(0);
  if (right == NewInt32(-1) || right == NewInt64(-1))
    return left;
  auto const node = new (zone()) IntBitAndNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntBitOr(Node* left, Node* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntBitOr(right, left);
  if (auto const present = FindBinaryNode(Opcode::IntBitOr, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(-1) || right == NewInt64(-1))
    return right;
  if (right == NewInt32(0) || right == NewInt64(0))
    return left;
  auto const node = new (zone()) IntBitOrNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntBitXor(Node* left, Node* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntBitXor(right, left);
  if (auto const present = FindBinaryNode(Opcode::IntBitXor, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(0) || right == NewInt64(0))
    return left;
  auto const node = new (zone()) IntBitXorNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntCmp(IntCondition condition, Node* left, Node* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  auto const node =
      new (zone()) IntCmpNode(bool_type(), condition, left, right);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewIntDiv(Node* left, Node* right) {
  if (auto const present = FindBinaryNode(Opcode::IntDiv, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  auto const node = new (zone()) IntDivNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntMul(Node* left, Node* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntAdd(right, left);
  if (auto const present = FindBinaryNode(Opcode::IntMul, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(0))
    return NewInt32(0);
  if (right == NewInt64(0))
    return NewInt64(0);
  if (right == NewInt32(1) || right == NewInt32(1))
    return left;
  auto const node = new (zone()) IntMulNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntMod(Node* left, Node* right) {
  if (auto const present = FindBinaryNode(Opcode::IntMod, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  auto const node = new (zone()) IntModNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntShl(Node* left, Node* right) {
  if (auto const present = FindBinaryNode(Opcode::IntShl, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK(type->is_integer()) << *left;
  DCHECK_EQ(right->output_type(), int32_type()) << *right;
  if (right == NewInt32(0))
    return left;
  auto const node = new (zone()) IntShlNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntShr(Node* left, Node* right) {
  if (auto const present = FindBinaryNode(Opcode::IntShr, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK(type->is_integer()) << *left;
  DCHECK_EQ(right->output_type(), int32_type()) << *right;
  if (right == NewInt32(0))
    return left;
  auto const node = new (zone()) IntShrNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Node* NodeFactory::NewIntSub(Node* left, Node* right) {
  if (auto const present = FindBinaryNode(Opcode::IntSub, left, right))
    return present;
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(0) || right == NewInt64(0))
    return left;
  auto const node = new (zone()) IntSubNode(type, left, right);
  node->set_id(NewNodeId());
  RememberBinaryNode(node);
  return node;
}

Control* NodeFactory::NewJump(Control* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) JumpNode(control_type(), control);
  node->set_id(NewNodeId());
  return node;
}

// Literal nodes
#define V(Name, name, data_type, ...)                   \
  Node* NodeFactory::New##Name(data_type data) {        \
    return node_cache_->New##Name(name##_type(), data); \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

size_t NodeFactory::NewNodeId() {
  return node_cache_->NewNodeId();
}

Node* NodeFactory::NewNull(Type* type) {
  return node_cache_->NewNull(type);
}

PhiOwnerNode* NodeFactory::NewMerge(const std::vector<Control*>& controls) {
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

PhiNode* NodeFactory::NewPhi(Type* output_type, PhiOwnerNode* owner) {
  DCHECK(owner->IsValidControl()) << *owner;
  auto const node = new (zone()) PhiNode(output_type, zone(), owner);
  node->set_id(NewNodeId());
  owner->phi_nodes_.AppendNode(node);
  return node;
}

Node* NodeFactory::NewReference(Type* type, AtomicString* name) {
  return node_cache_->NewReference(type, name);
}

Control* NodeFactory::NewRet(Control* control, Effect* effect, Node* data) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(data->IsValidData()) << *data;
  auto const node = new (zone()) RetNode(control_type(), control, effect, data);
  node->set_id(NewNodeId());
  return node;
}

Node* NodeFactory::NewStaticCast(Type* type, Node* input) {
  if (input->output_type() == type)
    return input;
  if (auto const present = FindUnaryNode(Opcode::StaticCast, type, input))
    return present;
  auto const node = new (zone()) StaticCastNode(type, input);
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

void NodeFactory::RememberBinaryNode(Node* node) {
  return node_cache_->RememberBinaryNode(node);
}

void NodeFactory::RememberFieldNode(Node* node, Node* input, size_t field) {
  return node_cache_->RememberFieldNode(node, input, field);
}

void NodeFactory::RememberUnaryNode(Node* node) {
  return node_cache_->RememberUnaryNode(node);
}

}  // namespace optimizer
}  // namespace elang
