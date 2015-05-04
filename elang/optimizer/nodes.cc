// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_visitor.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

// Control
Control::Control(Type* output_type) : Node(output_type) {
  DCHECK(output_type->is<ControlType>()) << output_type;
}

bool Control::IsControl() const {
  return true;
}

// Data
Data::Data(Type* output_type) : Node(output_type) {
}

bool Data::IsData() const {
  return true;
}

// Effect
Effect::Effect(Type* output_type) : Node(output_type) {
  DCHECK(output_type->is<EffectType>()) << output_type;
}

bool Effect::IsEffect() const {
  return true;
}

// EffectPhiNode
EffectPhiNode::EffectPhiNode(Type* output_type, Zone* zone, PhiOwnerNode* owner)
    : PhiNodeTemplate(output_type, zone, owner) {
  DCHECK(output_type->is<EffectType>()) << output_type;
}

// EntryNode
EntryNode::EntryNode(Type* output_type) : NodeTemplate(output_type) {
  DCHECK(output_type->is<ControlType>()) << output_type;
}

Type* EntryNode::parameters_type() const {
  return output_type()->as<ControlType>()->data_type();
}

Type* EntryNode::parameter_type(size_t index) const {
  auto const type = CheckedParameterTypeAt(index);
  DCHECK(type) << *output_type();
  return type;
}

Type* EntryNode::CheckedParameterTypeAt(size_t index) const {
  auto const type = parameters_type();
  if (auto const tuple_type = type->as<TupleType>())
    return index < tuple_type->size() ? tuple_type->get(index) : nullptr;
  return index ? nullptr : type;
}

// FloatCmpNode
FloatCmpNode::FloatCmpNode(Type* output_type,
                           FloatCondition condition,
                           Node* left,
                           Node* right)
    : NodeTemplate(output_type), condition_(condition) {
  InitInputAt(0, left);
  InitInputAt(1, right);
}

base::StringPiece FloatCmpNode::mnemonic() const {
  static const char* const mnemonics[] = {
#define V(Name, mnemonic, value) "fcmp_" mnemonic,
      FOR_EACH_OPTIMIZER_FLOAT_CONDITION(V)
#undef V
  };
  auto const it = std::begin(mnemonics) + static_cast<size_t>(condition_);
  return it < std::end(mnemonics) ? *it : "Invalid";
}

// FunctionReferenceNode
FunctionReferenceNode::FunctionReferenceNode(Type* output_type,
                                             Function* function)
    : NodeTemplate(output_type), function_(function) {
  DCHECK_EQ(output_type->as<PointerType>()->pointee(),
            function->function_type());
}

// GetNode
GetNode::GetNode(Type* output_type, Tuple* input, size_t field)
    : ProjectionNodeTemplate(output_type, input, field) {
  DCHECK(!output_type->is<ControlType>() && !output_type->is<EffectType>())
      << output_type;
}

// Input
Input::Input() : from_(nullptr), to_(nullptr) {
}

void Input::Init(Node* from, Node* to) {
  DCHECK(from);
  DCHECK(to);
  DCHECK(!from_) << from_ << " " << from;
  DCHECK(!to_) << to_ << " " << from;
  from_ = from;
  to_ = to;
  to_->Use(this);
}

void Input::Reset() {
  DCHECK(to_) << from_;
  to_->Unuse(this);
  to_ = nullptr;
}

void Input::SetTo(Node* new_to) {
  DCHECK(to_) << from_;
  to_->Unuse(this);
  new_to->Use(this);
  to_ = new_to;
}

// InputHolder
InputHolder::InputHolder() {
}

InputHolder::~InputHolder() {
}

// Literal
Literal::Literal(Type* output_type) : Data(output_type) {
}

bool Literal::IsLiteral() const {
  return true;
}

// Node::InputIterator
Node::InputIterator::InputIterator(const Node* node, size_t current)
    : current_(current), node_(node) {
}

Node::InputIterator::InputIterator(const InputIterator& other)
    : InputIterator(other.node_, other.current_) {
}

Node::InputIterator::~InputIterator() {
}

Node::InputIterator& Node::InputIterator::operator=(
    const InputIterator& other) {
  node_ = other.node_;
  current_ = other.current_;
  return *this;
}

Node::InputIterator& Node::InputIterator::operator++() {
  ++current_;
  return *this;
}

Node* Node::InputIterator::operator*() const {
  return node_->input(current_);
}

Node* Node::InputIterator::operator->() const {
  return operator*();
}

bool Node::InputIterator::operator==(const InputIterator& other) const {
  return node_ == other.node_ && current_ == other.current_;
}

bool Node::InputIterator::operator!=(const InputIterator& other) const {
  return !operator==(other);
}

// Inputs
Node::Inputs::Inputs(const Node* node) : node_(node) {
}

Node::Inputs::~Inputs() {
}

Node::Inputs& Node::Inputs::operator=(const Inputs& other) {
  node_ = other.node_;
  return *this;
}

Node::InputIterator Node::Inputs::begin() {
  return InputIterator(node_, 0);
}

Node::InputIterator Node::Inputs::end() {
  return InputIterator(node_, node_->CountInputs());
}

// IntCmpNode
IntCmpNode::IntCmpNode(Type* output_type,
                       IntCondition condition,
                       Node* left,
                       Node* right)
    : NodeTemplate(output_type), condition_(condition) {
  InitInputAt(0, left);
  InitInputAt(1, right);
}

base::StringPiece IntCmpNode::mnemonic() const {
  static const char* const mnemonics[] = {
#define V(Name, mnemonic, value) "cmp_" mnemonic,
      FOR_EACH_OPTIMIZER_INTEGER_CONDITION(V)
#undef V
  };
  auto const it = std::begin(mnemonics) + static_cast<size_t>(condition_);
  return it < std::end(mnemonics) ? *it : "Invalid";
}

// Literals
#define V(Name, mnemonic, data_type)                        \
  Name##Node::Name##Node(Type* output_type, data_type data) \
      : LiteralNodeTemplate(output_type, data) {            \
    DCHECK(output_type->is<Name##Type>()) << output_type;   \
  }
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

// LoopNode
LoopNode::LoopNode(Type* output_type, Zone* zone)
    : PhiOwnerNode(output_type, zone) {
}

// MergeNode
MergeNode::MergeNode(Type* output_type, Zone* zone)
    : PhiOwnerNode(output_type, zone) {
}

// Node
Node::Node(Type* output_type) : id_(0), output_type_(output_type) {
}

Control* Node::control(size_t index) const {
  auto const control = input(index)->as<Control>();
  DCHECK(control) << *this << " " << index;
  return control;
}

Node* Node::input(size_t index) const {
  return InputAt(index)->value();
}

Node::Inputs Node::inputs() const {
  return Inputs(this);
}

base::StringPiece Node::mnemonic() const {
  static const char* mnemonics[] = {
#define V(Name, mnemonic, ...) mnemonic,
      FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V
  };
  auto const it = std::begin(mnemonics) + static_cast<size_t>(opcode());
  return it < std::end(mnemonics) ? *it : "invalid_opcode";
}

#define V(Name, ...) \
  Opcode Name##Node::opcode() const { return Opcode::Name; }
FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V

void Node::set_id(size_t id) {
  id_ = base::checked_cast<uint32_t>(id);
}

// Node::Accept
#define V(Name, ...) \
  void Name##Node::Accept(NodeVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V

void Node::InitInputAt(size_t index, Node* value) {
  InputAt(index)->Init(this, value);
}

bool Node::IsBlockEnd() const {
  static bool block_end[static_cast<size_t>(Opcode::NumberOfOpcodes)];
  if (!block_end[static_cast<size_t>(Opcode::Entry)]) {
#define V(Name) block_end[static_cast<size_t>(Opcode::Name)] = true;
    FOR_EACH_OPTIMIZER_BLOCK_END_NODE(V)
#undef V
  }
  return block_end[static_cast<size_t>(opcode())];
}

bool Node::IsBlockLabel() const {
  static bool block_label[static_cast<size_t>(Opcode::NumberOfOpcodes)];
  if (!block_label[static_cast<size_t>(Opcode::IfFalse)]) {
#define V(Name) block_label[static_cast<size_t>(Opcode::Name)] = true;
    FOR_EACH_OPTIMIZER_BLOCK_LABEL_NODE(V)
#undef V
  }
  return block_label[static_cast<size_t>(opcode())];
}

bool Node::IsBlockStart() const {
  static bool block_start[static_cast<size_t>(Opcode::NumberOfOpcodes)];
  if (!block_start[static_cast<size_t>(Opcode::Entry)]) {
#define V(Name) block_start[static_cast<size_t>(Opcode::Name)] = true;
    FOR_EACH_OPTIMIZER_BLOCK_START_NODE(V)
#undef V
  }
  return block_start[static_cast<size_t>(opcode())];
}

bool Node::IsControl() const {
  return false;
}

bool Node::IsControlEffect() const {
  auto const opcode = this->opcode();
  return opcode == Opcode::Call || opcode == Opcode::Entry;
}

bool Node::IsData() const {
  return false;
}

bool Node::IsEffect() const {
  return false;
}

bool Node::IsLiteral() const {
  return false;
}

bool Node::IsTuple() const {
  return false;
}

bool Node::IsUsed() const {
  return !use_edges().empty() || opcode() == Opcode::Exit;
}

bool Node::IsValidControl() const {
  return IsControl() && id_;
}

bool Node::IsValidControlAt(size_t field) const {
  auto const type = output_type_->as<TupleType>();
  return id_ && type && type->get(field)->is<ControlType>();
}

bool Node::IsValidData() const {
  return !CountInputs() || (IsData() && id_);
}

bool Node::IsValidEffect() const {
  return IsEffect() && id_;
}

bool Node::IsValidEffectAt(size_t field) const {
  auto const type = output_type_->as<TupleType>();
  return id_ && type && type->get(field)->is<EffectType>();
}

void Node::ResetInputAt(size_t index) {
  InputAt(index)->Reset();
}

Node* Node::SelectUser(Opcode opcode) const {
  for (auto const edge : use_edges()) {
    auto const user = edge->from();
    if (user->opcode() == opcode)
      return user;
  }
  return nullptr;
}

Node* Node::SelectUserIfOne() const {
  auto single_user = static_cast<Node*>(nullptr);
  for (auto const edge : use_edges()) {
    auto const user = edge->from();
    if (single_user)
      return nullptr;
    single_user = user;
  }
  return single_user;
}

void Node::SetInputAt(size_t index, Node* value) {
  InputAt(index)->SetTo(value);
}

void Node::Use(Input* input) {
  use_edges_.AppendNode(input);
}

void Node::Unuse(Input* input) {
  use_edges_.RemoveNode(input);
}

// Node NodeLayout implementation
size_t Node::field() const {
  NOTREACHED() << *this;
  return 0;
}

bool Node::has_field() const {
  return false;
}

void Node::AppendInput(Node* node) {
  NOTREACHED() << *this << " " << *node;
}

Input* Node::InputAt(size_t index) const {
  NOTREACHED() << *this << " " << index;
  return nullptr;
}

bool Node::IsVariadic() const {
  return false;
}

// NodeLayout
NodeLayout::NodeLayout() {
}

NodeLayout::~NodeLayout() {
}

// NullNode
NullNode::NullNode(Type* output_type) : NodeTemplate(output_type) {
}

// ParameterNode
ParameterNode::ParameterNode(Type* output_type, EntryNode* input, size_t field)
    : NodeTemplate(output_type), field_(field) {
  DCHECK_EQ(input->parameter_type(field), output_type) << *output_type << " "
                                                       << *input;
  InitInputAt(0, input);
}

// PhiInputHolder
PhiInputHolder::PhiInputHolder(Control* control) : control_(control) {
}

PhiInputHolder::~PhiInputHolder() {
}

// PhiNode
PhiNode::PhiNode(Type* output_type, Zone* zone, PhiOwnerNode* owner)
    : PhiNodeTemplate(output_type, zone, owner) {
}

// PhiOwnerNode
PhiOwnerNode::PhiOwnerNode(Type* output_type, Zone* zone)
    : VariadicNodeTemplate(output_type, zone), effect_phi_(nullptr) {
}

void PhiOwnerNode::set_effect_phi(EffectPhiNode* effect_phi) {
  DCHECK(!effect_phi_) << this << " " << effect_phi_;
  DCHECK(effect_phi) << this;
  effect_phi_ = effect_phi;
}

// ReferenceNode
ReferenceNode::ReferenceNode(Type* output_type, AtomicString* name)
    : NodeTemplate(output_type), name_(name) {
}

// Simple nodes
#define V(Name, ...)                                      \
  Name##Node::Name##Node(Type* output_type, Node* input0) \
      : NodeTemplate(output_type) {                       \
    InitInputAt(0, input0);                               \
  }
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_1(V)
#undef V

#define V(Name, ...)                                                    \
  Name##Node::Name##Node(Type* output_type, Node* input0, Node* input1) \
      : NodeTemplate(output_type) {                                     \
    InitInputAt(0, input0);                                             \
    InitInputAt(1, input1);                                             \
  }
FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V)
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_2(V)
#undef V

#define V(Name, ...)                                                    \
  Name##Node::Name##Node(Type* output_type, Node* input0, Node* input1, \
                         Node* input2)                                  \
      : NodeTemplate(output_type) {                                     \
    InitInputAt(0, input0);                                             \
    InitInputAt(1, input1);                                             \
    InitInputAt(2, input2);                                             \
  }
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_3(V)
#undef V

#define V(Name, ...)                                                    \
  Name##Node::Name##Node(Type* output_type, Node* input0, Node* input1, \
                         Node* input2, Node* input3)                    \
      : NodeTemplate(output_type) {                                     \
    InitInputAt(0, input0);                                             \
    InitInputAt(1, input1);                                             \
    InitInputAt(2, input2);                                             \
    InitInputAt(3, input3);                                             \
  }
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_4(V)
#undef V

// SizeOf
SizeOfNode::SizeOfNode(Type* uintptr_type, Type* type)
    : NodeTemplate(uintptr_type), type_operand_(type) {
  DCHECK(output_type()->is<UIntPtrType>()) << output_type();
}

// Tuple
Tuple::Tuple(Type* output_type) : Node(output_type) {
  DCHECK(output_type->is<TupleType>()) << output_type;
}

bool Tuple::IsData() const {
  auto const tuple_type = output_type()->as<TupleType>();
  for (auto const component : tuple_type->components()) {
    if (component->is<ControlType>() || component->is<EffectType>())
      return false;
  }
  return true;
}

bool Tuple::IsTuple() const {
  return true;
}

// Variable nodes
#define V(Name, ...)                                    \
  Name##Node::Name##Node(Type* output_type, Zone* zone) \
      : VariadicNodeTemplate(output_type, zone) {}
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_V(V)
#undef V

// VoidNode
VoidNode::VoidNode(Type* output_type) : NodeTemplate(output_type) {
  DCHECK(output_type->is<VoidType>()) << output_type;
}

}  // namespace optimizer
}  // namespace elang
