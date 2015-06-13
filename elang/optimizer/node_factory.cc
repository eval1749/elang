// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "elang/optimizer/node_factory.h"

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
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

  Data* value() const {
    DCHECK(value_);
    return value_;
  }

 private:
  Data* value_;

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

int64_t AsInt64(const Data* literal) {
  DCHECK(literal->output_type()->is_signed());
  if (auto const i8 = literal->as<Int8Node>())
    return i8->data();
  if (auto const i16 = literal->as<Int16Node>())
    return i16->data();
  if (auto const i32 = literal->as<Int32Node>())
    return i32->data();
  if (auto const i64 = literal->as<Int64Node>())
    return i64->data();
  if (auto const i64 = literal->as<IntPtrNode>())
    return i64->data();
  NOTREACHED() << *literal;
  return 0;
}

uint64_t AsUInt64(const Data* literal) {
  DCHECK(literal->output_type()->is_signed());
  if (auto const u8 = literal->as<UInt8Node>())
    return u8->data();
  if (auto const u16 = literal->as<UInt16Node>())
    return u16->data();
  if (auto const u32 = literal->as<UInt32Node>())
    return u32->data();
  if (auto const u64 = literal->as<UInt64Node>())
    return u64->data();
  if (auto const u64 = literal->as<UIntPtrNode>())
    return u64->data();
  NOTREACHED() << *literal;
  return 0;
}

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

Data* NodeFactory::CalculateConstant(IntCondition condition,
                                     Data* left,
                                     Data* right) {
  DCHECK(left->IsLiteral()) << *left;
  DCHECK(right->IsLiteral()) << *right;
  DCHECK_EQ(left->output_type(), right->output_type());
  if (left->output_type()->is_signed()) {
    auto const left_i64 = AsInt64(left);
    auto const right_i64 = AsInt64(right);
    switch (condition) {
      case IntCondition::Equal:
        return left_i64 == right_i64 ? true_value() : false_value();
      case IntCondition::NotEqual:
        return left_i64 != right_i64 ? true_value() : false_value();
      case IntCondition::SignedGreaterThan:
        return left_i64 > right_i64 ? true_value() : false_value();
      case IntCondition::SignedGreaterThanOrEqual:
        return left_i64 >= right_i64 ? true_value() : false_value();
      case IntCondition::SignedLessThan:
        return left_i64 < right_i64 ? true_value() : false_value();
      case IntCondition::SignedLessThanOrEqual:
        return left_i64 <= right_i64 ? true_value() : false_value();
    }
    NOTREACHED() << "Invalid condition: " << condition << left << right;
    return nullptr;
  }
  auto const left_u64 = AsInt64(left);
  auto const right_u64 = AsInt64(right);
  switch (condition) {
    case IntCondition::Equal:
      return left_u64 == right_u64 ? true_value() : false_value();
    case IntCondition::NotEqual:
      return left_u64 != right_u64 ? true_value() : false_value();
    case IntCondition::UnsignedGreaterThan:
      return left_u64 > right_u64 ? true_value() : false_value();
    case IntCondition::UnsignedGreaterThanOrEqual:
      return left_u64 >= right_u64 ? true_value() : false_value();
    case IntCondition::UnsignedLessThan:
      return left_u64 < right_u64 ? true_value() : false_value();
    case IntCondition::UnsignedLessThanOrEqual:
      return left_u64 <= right_u64 ? true_value() : false_value();
  }
  NOTREACHED() << "Invalid condition: " << condition << left << right;
  return nullptr;
}

Data* NodeFactory::DefaultValueOf(Type* type) {
  if (type == void_type())
    return void_value_;
  DefaultValueFactory factory(node_cache_.get());
  type->Accept(&factory);
  return factory.value();
}

Control* NodeFactory::NewCall(Control* control,
                              Effect* effect,
                              Data* callee,
                              Node* arguments) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK(callee->IsValidData()) << *callee;
  DCHECK(callee->output_type()->is<FunctionType>()) << *callee;
  DCHECK(arguments->IsValidData()) << *arguments;
  auto const output_type =
      NewControlType(callee->output_type()->as<FunctionType>()->return_type());
  auto const node =
      new (zone()) CallNode(output_type, control, effect, callee, arguments);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewDynamicCast(Type* type, Data* input) {
  if (input->output_type() == type)
    return input;
  auto const node = new (zone()) DynamicCastNode(type, input);
  node->set_id(NewNodeId());
  return node;
}

EffectPhiNode* NodeFactory::NewEffectPhi(PhiOwnerNode* owner) {
  DCHECK(owner->IsValidControl()) << *owner;
  auto const node = new (zone()) EffectPhiNode(effect_type(), zone(), owner);
  node->set_id(NewNodeId());
  owner->set_effect_phi(node);
  return node;
}

Data* NodeFactory::NewElement(Data* array, Node* indexes) {
  auto const array_pointer_type = array->output_type()->as<PointerType>();
  DCHECK(array_pointer_type) << *array->output_type();
  auto const array_type = array_pointer_type->pointee()->as<ArrayType>();
  DCHECK(array_type) << *array->output_type();
#ifndef NDEBUG
  if (array_type->rank() == 1) {
    DCHECK_EQ(int32_type(), indexes->output_type());
  } else if (auto const indexes_type = indexes->as<TupleType>()) {
    DCHECK_EQ(array_type->rank(), indexes_type->size());
    for (auto const type : indexes_type->components())
      DCHECK_EQ(int32_type(), type);
  }
#endif
  auto const output_type = NewPointerType(array_type->element_type());
  auto const node = new (zone()) ElementNode(output_type, array, indexes);
  node->set_id(NewNodeId());
  return node;
}

EntryNode* NodeFactory::NewEntry(Type* parameters_type) {
  auto const output_type = NewControlType(parameters_type);
  auto const node = new (zone()) EntryNode(output_type);
  node->set_id(NewNodeId());
  return node;
}

ExitNode* NodeFactory::NewExit(Control* control) {
  DCHECK(control->IsValidControl()) << *control;
  auto const node = new (zone()) ExitNode(control_type(), control);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewField(Type* field_type,
                            Data* instance,
                            Data* field_name) {
  auto const instance_pointer_type = instance->output_type()->as<PointerType>();
  DCHECK(instance_pointer_type) << *instance->output_type();
  auto const clazz = instance_pointer_type->pointee()->as<ExternalType>();
  DCHECK(clazz) << instance->output_type();
  auto const output_type = NewPointerType(field_type);
  auto const node = new (zone()) FieldNode(output_type, instance, field_name);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewFloatAdd(Data* left, Data* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewFloatAdd(right, left);
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatAddNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewFloatCmp(FloatCondition condition,
                               Data* left,
                               Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node =
      new (zone()) FloatCmpNode(bool_type(), condition, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewFloatDiv(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatDivNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewFloatMul(Data* left, Data* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewFloatAdd(right, left);
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatMulNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewFloatMod(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatModNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewFloatSub(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_float());
  auto const node = new (zone()) FloatSubNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewFunctionReference(Function* function) {
  auto const output_type = NewPointerType(function->function_type());
  return node_cache_->NewFunctionReference(output_type, function);
}

Data* NodeFactory::NewGet(Tuple* input, size_t field) {
  DCHECK(input->id() || input->IsLiteral()) << *input << " " << field;
  auto const output_type = input->output_type()->as<TupleType>()->get(field);
  auto const node = new (zone()) GetNode(output_type, input, field);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewGetData(Control* input) {
  auto const data_type = input->output_type()->as<ControlType>()->data_type();
  DCHECK(!data_type->is<VoidType>()) << *data_type;
  auto const node = new (zone()) GetDataNode(data_type, input);
  node->set_id(NewNodeId());
  return node;
}

Effect* NodeFactory::NewGetEffect(Control* input) {
  auto const node = new (zone()) GetEffectNode(effect_type(), input);
  node->set_id(NewNodeId());
  return node;
}

Tuple* NodeFactory::NewGetTuple(Control* input) {
  auto const data_type = input->output_type()->as<ControlType>()->data_type();
  DCHECK(data_type->is<TupleType>());
  auto const node = new (zone()) GetTupleNode(data_type, input);
  node->set_id(NewNodeId());
  return node;
}

Control* NodeFactory::NewIf(Control* control, Data* data) {
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

Data* NodeFactory::NewIntAdd(Data* left, Data* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntAdd(right, left);
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer()) << *left << " " << *right;
  if (right == NewInt32(0) || right == NewInt64(0))
    return left;
  auto const node = new (zone()) IntAddNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntBitAnd(Data* left, Data* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntBitAnd(right, left);
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
  return node;
}

Data* NodeFactory::NewIntBitOr(Data* left, Data* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntBitOr(right, left);
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(-1) || right == NewInt64(-1))
    return right;
  if (right == NewInt32(0) || right == NewInt64(0))
    return left;
  auto const node = new (zone()) IntBitOrNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntBitXor(Data* left, Data* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntBitXor(right, left);
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(0) || right == NewInt64(0))
    return left;
  auto const node = new (zone()) IntBitXorNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntCmp(IntCondition condition, Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer() || type->is<PointerType>()) << *type;
#if NDEBUG
  if (type->is_signed()) {
    DCHECK(condition != IntCondition::UnsignedGreaterThan &&
           condition != IntCondition::UnsignedGreaterThanOrEqual &&
           condition != IntCondition::UnsignedLessThan &&
           condition != IntCondition::UnsignedLessThanOrEqual);
  } else {
    DCHECK(condition != IntCondition::SignedGreaterThan &&
           condition != IntCondition::SignedGreaterThanOrEqual &&
           condition != IntCondition::SignedLessThan &&
           condition != IntCondition::SignedLessThanOrEqual);
  }
#endif
  if (left->IsLiteral() && right->IsLiteral())
    return CalculateConstant(condition, left, right);
  if (left->IsLiteral())
    return NewIntCmp(CommuteCondition(condition), right, left);
  auto const node =
      new (zone()) IntCmpNode(bool_type(), condition, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntDiv(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer()) << *left << " " << *right;
  DCHECK(type->is_signed()) << *left << " " << *right;
  auto const node = new (zone()) IntDivNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntMul(Data* left, Data* right) {
  if (left->IsLiteral() && !right->IsLiteral())
    return NewIntAdd(right, left);
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer()) << *left << " " << *right;
  if (right == NewInt32(0))
    return NewInt32(0);
  if (right == NewInt64(0))
    return NewInt64(0);
  if (right == NewInt32(1) || right == NewInt32(1))
    return left;
  auto const node = new (zone()) IntMulNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntMod(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer()) << *left << " " << *right;
  DCHECK(type->is_signed()) << *left << " " << *right;
  auto const node = new (zone()) IntModNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntShl(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK(type->is_integer()) << *left;
  DCHECK_EQ(right->output_type(), int32_type()) << *right;
  if (right == NewInt32(0))
    return left;
  auto const node = new (zone()) IntShlNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntShr(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK(type->is_integer()) << *left;
  DCHECK_EQ(right->output_type(), int32_type()) << *right;
  if (right == NewInt32(0))
    return left;
  auto const node = new (zone()) IntShrNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewIntSub(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer());
  if (right == NewInt32(0) || right == NewInt64(0))
    return left;
  auto const node = new (zone()) IntSubNode(type, left, right);
  node->set_id(NewNodeId());
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
  Data* NodeFactory::New##Name(data_type data) {        \
    return node_cache_->New##Name(name##_type(), data); \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V

LoopNode* NodeFactory::NewLoop() {
  auto const node = new (zone()) LoopNode(control_type(), zone());
  node->set_id(NewNodeId());
  return node;
}

size_t NodeFactory::NewNodeId() {
  return node_cache_->NewNodeId();
}

Data* NodeFactory::NewNull(Type* type) {
  return node_cache_->NewNull(type);
}

Data* NodeFactory::NewLength(Data* array, size_t rank) {
  auto const array_pointer_type = array->output_type()->as<PointerType>();
  DCHECK(array_pointer_type) << *array->output_type();
  auto const array_type = array_pointer_type->pointee()->as<ArrayType>();
  DCHECK(array_type) << *array->output_type();
  DCHECK_LT(rank, array_type->rank());
  auto const rank_node = NewInt32(base::checked_cast<int32_t>(rank));
  auto const node = new (zone()) LengthNode(int32_type(), array, rank_node);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewLoad(Effect* effect, Data* base_pointer, Data* pointer) {
  auto const pointer_type = pointer->output_type()->as<PointerType>();
  DCHECK(pointer_type) << *pointer;
  auto const node = new (zone())
      LoadNode(pointer_type->pointee(), effect, base_pointer, pointer);
  node->set_id(NewNodeId());
  return node;
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

Data* NodeFactory::NewParameter(EntryNode* input, size_t field) {
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

Data* NodeFactory::NewReference(Type* type, AtomicString* name) {
  return node_cache_->NewReference(type, name);
}

Control* NodeFactory::NewRet(Control* control, Effect* effect, Data* data) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(data->IsValidData()) << *data;
  auto const node = new (zone()) RetNode(control_type(), control, effect, data);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewSizeOf(Type* type) {
  return node_cache_->NewSizeOf(type);
}

Data* NodeFactory::NewStaticCast(Type* type, Data* input) {
  if (input->output_type() == type)
    return input;
  auto const node = new (zone()) StaticCastNode(type, input);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewString(base::StringPiece16 data) {
  return node_cache_->NewString(string_type(), data);
}

Effect* NodeFactory::NewStore(Effect* effect,
                              Data* anchor,
                              Data* pointer,
                              Data* value) {
  auto const node =
      new (zone()) StoreNode(effect_type(), effect, anchor, pointer, value);
  node->set_id(NewNodeId());
  return node;
}

Tuple* NodeFactory::NewTuple(const std::vector<Node*>& inputs) {
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

Data* NodeFactory::NewUIntDiv(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer()) << *left << " " << *right;
  DCHECK(type->is_unsigned()) << *left << " " << *right;
  auto const node = new (zone()) UIntDivNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

Data* NodeFactory::NewUIntMod(Data* left, Data* right) {
  auto const type = left->output_type();
  DCHECK_EQ(type, right->output_type()) << *left << " " << *right;
  DCHECK(type->is_integer()) << *left << " " << *right;
  DCHECK(type->is_unsigned()) << *left << " " << *right;
  auto const node = new (zone()) UIntModNode(type, left, right);
  node->set_id(NewNodeId());
  return node;
}

}  // namespace optimizer
}  // namespace elang
