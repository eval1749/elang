// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"
#include "elang/hir/value_visitor.h"

namespace elang {
namespace hir {

#define V(Name, ...) \
  void Name::Accept(ValueVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_HIR_VALUE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// UseDefNode
//
UseDefNode::UseDefNode() : instruction_(nullptr), value_(nullptr) {
}

void UseDefNode::Init(Instruction* instruction, Value* value) {
  DCHECK(!instruction_);
  DCHECK(!value_);
  DCHECK(value);
  instruction_ = instruction;
  value_ = value;
  value_->Use(this);
}

void UseDefNode::SetValue(Value* new_value) {
  if (value_)
    value_->Unuse(this);
  new_value->Use(this);
}

//////////////////////////////////////////////////////////////////////
//
// Value
//
Value::Value(Type* type) : type_(type) {
}

void Value::Use(UseDefNode* value_holder) {
  use_def_list_.AppendNode(value_holder);
}

void Value::Unuse(UseDefNode* value_holder) {
  use_def_list_.RemoveNode(value_holder);
}

//////////////////////////////////////////////////////////////////////
//
// Literal
//
#define V(Name, name, c_type, ...)                             \
  c_type Literal::name##_value() const {                       \
    NOTREACHED();                                              \
    return c_type();                                           \
  }                                                            \
  c_type Name##Literal::name##_value() const { return data_; } \
  Name##Literal::Name##Literal(Type* type, c_type data)        \
      : Literal(type), data_(data) {}
FOR_EACH_HIR_LITERAL_VALUE(V)
#undef V

Literal::Literal(Type* type) : Value(type) {
}

NullLiteral::NullLiteral(Type* type) : Literal(type) {
}

VoidLiteral::VoidLiteral(VoidType* type, int data) : Literal(type) {
  __assume(!data);
}

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
BasicBlock::BasicBlock(Factory* factory)
    : Value(factory->GetVoidType()), function_(nullptr), id_(0) {
}

Instruction* BasicBlock::first_instruction() const {
  return instructions_.first_node();
}

Instruction* BasicBlock::last_instruction() const {
  return instructions_.last_node();
}

//////////////////////////////////////////////////////////////////////
//
// Function
//
Function::Function(Factory* factory, FunctionType* type) : Value(type) {
  Editor function(factory, this);
}

BasicBlock* Function::entry_block() const {
  auto const block = basic_blocks_.first_node();
  DCHECK(block->first_instruction()->is<EntryInstruction>());
  return block;
}

BasicBlock* Function::exit_block() const {
  auto const block = basic_blocks_.last_node();
  DCHECK(block->first_instruction()->is<ExitInstruction>());
  return block;
}

FunctionType* Function::function_type() const {
  return type()->as<FunctionType>();
}

}  // namespace hir
}  // namespace elang
