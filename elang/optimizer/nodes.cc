// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_visitor.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

// FloatCmpNode
FloatCmpNode::FloatCmpNode(Type* output_type, FloatCondition condition)
    : NodeTemplate(output_type), condition_(condition) {
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

// Function
Function::Function(FunctionType* function_type,
                   Node* entry_node,
                   Node* exit_node)
    : entry_node_(entry_node),
      function_type_(function_type),
      exit_node_(exit_node) {
}

Type* Function::parameters_type() const {
  return function_type()->parameters_type();
}

Type* Function::return_type() const {
  return function_type()->return_type();
}

// FunctionReferenceNode
FunctionReferenceNode::FunctionReferenceNode(Type* output_type,
                                             Function* function)
    : NodeTemplate(output_type), function_(function) {
  DCHECK_EQ(output_type->as<PointerType>()->pointee(),
            function->function_type());
}

// Input
Input::Input(const Input& other) : owner_(other.owner_), value_(other.value_) {
}

Input::Input() : owner_(nullptr), value_(nullptr) {
}

Input& Input::operator=(const Input& other) {
  owner_ = other.owner_;
  value_ = other.value_;
  return *this;
}

void Input::Init(Node* owner, Node* value) {
  DCHECK(owner);
  DCHECK(value);
  DCHECK(!owner_);
  DCHECK(!value_);
  owner_ = owner;
  value_ = value;
  value_->Use(this);
}

void Input::Reset() {
  DCHECK(!value_) << "Already reset";
  value_->Unuse(this);
  value_ = nullptr;
}

void Input::SetValue(Node* new_value) {
  DCHECK(value_);
  value_->Unuse(this);
  new_value->Use(this);
  value_ = new_value;
}

// InputIterator
InputIterator::InputIterator(const Node* node, size_t current)
    : current_(current), node_(node) {
}

InputIterator::InputIterator(const InputIterator& other)
    : InputIterator(other.node_, other.current_) {
}

InputIterator::~InputIterator() {
}

InputIterator& InputIterator::operator=(const InputIterator& other) {
  node_ = other.node_;
  current_ = other.current_;
  return *this;
}

InputIterator& InputIterator::operator++() {
  ++current_;
  return *this;
}

Node* InputIterator::operator*() const {
  return node_->input(current_);
}

Node* InputIterator::operator->() const {
  return operator*();
}

bool InputIterator::operator==(const InputIterator& other) const {
  return node_ == other.node_ && current_ == other.current_;
}

bool InputIterator::operator!=(const InputIterator& other) const {
  return !operator==(other);
}

// Inputs
Inputs::Inputs(const Node* node) : node_(node) {
}

Inputs::~Inputs() {
}

Inputs& Inputs::operator=(const Inputs& other) {
  node_ = other.node_;
  return *this;
}

InputIterator Inputs::begin() {
  return InputIterator(node_, 0);
}

InputIterator Inputs::end() {
  return InputIterator(node_, node_->CountInputs());
}

// IntCmpNode
IntCmpNode::IntCmpNode(Type* output_type, IntCondition condition)
    : NodeTemplate(output_type), condition_(condition) {
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
      : NodeTemplate(output_type), data_(data) {            \
    DCHECK(output_type->is<Name##Type>());                  \
  }                                                         \
  bool Name##Node::IsLiteral() const { return true; }
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

// Node
Node::Node(Type* output_type) : id_(0), output_type_(output_type) {
}

Node* Node::input(size_t index) const {
  return InputAt(index)->value();
}

Inputs Node::inputs() const {
  return Inputs(this);
}

base::StringPiece Node::mnemonic() const {
  static const char* mnemonics[] = {
#define V(Name, ...) #Name,
      FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V
  };
  auto const it = std::begin(mnemonics) + static_cast<size_t>(opcode());
  return it < std::end(mnemonics) ? *it : "INVALID";
}

#define V(Name, ...) \
  Opcode Name##Node::opcode() const { return Opcode::Name##Node; }
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

bool Node::IsControl() const {
  if (output_type_->is<ControlType>())
    return true;
  if (auto const tuple_type = output_type_->as<TupleType>())
    return tuple_type->get(0)->is<ControlType>();
  return false;
}

bool Node::IsEffect() const {
  if (output_type_->is<EffectType>())
    return true;
  if (auto const tuple_type = output_type_->as<TupleType>())
    return tuple_type->get(1)->is<EffectType>();
  return false;
}

bool Node::IsLiteral() const {
  return false;
}

void Node::ResetInputAt(size_t index) {
  InputAt(index)->Reset();
}

void Node::SetInputAt(size_t index, Node* value) {
  InputAt(index)->SetValue(value);
}

void Node::Use(Input* input) {
  use_def_list_.AppendNode(input);
}

void Node::Unuse(Input* input) {
  use_def_list_.RemoveNode(input);
}

// NullNode
NullNode::NullNode(Type* output_type) : NodeTemplate(output_type) {
}

// Simple nodes
#define V(Name, ...) \
  Name##Node::Name##Node(Type* output_type) : NodeTemplate(output_type) {}
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_0(V)
#undef V

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

// Variable nodes
#define V(Name, ...)                                    \
  Name##Node::Name##Node(Type* output_type, Zone* zone) \
      : VariableInputsNode(output_type, zone) {}
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_V(V)
#undef V

// VariableInputsNode
VariableInputsNode::VariableInputsNode(Type* output_type, Zone* zone)
    : Node(output_type), inputs_(zone) {
}

void VariableInputsNode::AppendInput(Node* value) {
  inputs_.emplace_back();
  InitInputAt(inputs_.size() - 1, value);
}

// VoidNode
VoidNode::VoidNode(Type* output_type) : NodeTemplate(output_type) {
  DCHECK(output_type->is<VoidType>());
}

}  // namespace optimizer
}  // namespace elang
