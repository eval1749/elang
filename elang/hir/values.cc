// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/editor.h"
#include "elang/hir/error_code.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"
#include "elang/hir/value_visitor.h"

namespace elang {
namespace hir {

// boilerplate member functions.
#define V(Name, ...) \
  void Name::Accept(ValueVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_HIR_VALUE(V)
#undef V

#define V(Name, name, c_type, ...)                      \
  Name##Literal::Name##Literal(Type* type, c_type data) \
      : Literal(type), data_(data) {}
FOR_EACH_HIR_LITERAL_VALUE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
BasicBlock::BasicBlock(Factory* factory)
    : Value(factory->void_type()), function_(nullptr), id_(0) {
}

Instruction* BasicBlock::first_instruction() const {
  return instructions_.first_node();
}

Instruction* BasicBlock::last_instruction() const {
  return instructions_.last_node();
}

PhiInstructionList BasicBlock::phi_instructions() const {
  return PhiInstructionList(phi_instructions_);
}

BasicBlockPredecessors BasicBlock::predecessors() const {
  return BasicBlockPredecessors(this);
}

BasicBlockSuccessors BasicBlock::successors() const {
  return BasicBlockSuccessors(this);
}

bool BasicBlock::HasPredecessor() const {
  return predecessors().begin() != predecessors().end();
}

// BasicBlockPredecessors
BasicBlockPredecessors::BasicBlockPredecessors(const BasicBlock* basic_block)
    : basic_block_(basic_block) {
}

BasicBlockPredecessors::Iterator BasicBlockPredecessors::begin() const {
  return Iterator(basic_block_->users().begin());
}

BasicBlockPredecessors::Iterator BasicBlockPredecessors::end() const {
  return Iterator(basic_block_->users().end());
}

// BasicBlockPredecessors::Iterator
BasicBlockPredecessors::Iterator::Iterator(const UseDefList::Iterator& iterator)
    : IteratorOnIterator(iterator) {
}

BasicBlock* BasicBlockPredecessors::Iterator::operator*() const {
  return (*iterator())->instruction()->basic_block();
}

// BasicBlockSuccessors
BasicBlockSuccessors::BasicBlockSuccessors(const BasicBlock* basic_block)
    : basic_block_(basic_block) {
}

BasicBlockSuccessors::Iterator BasicBlockSuccessors::begin() const {
  auto inputs = basic_block_->last_instruction()->inputs();
  auto it = inputs.begin();
  while (it != inputs.end() && !(*it)->is<BasicBlock>())
    ++it;
  return Iterator(it);
}

BasicBlockSuccessors::Iterator BasicBlockSuccessors::end() const {
  return Iterator(basic_block_->last_instruction()->inputs().end());
}

BasicBlockSuccessors::Iterator::Iterator(const OperandIterator& iterator)
    : IteratorOnIterator(iterator) {
}

BasicBlock* BasicBlockSuccessors::Iterator::operator*() const {
  return (*iterator())->as<BasicBlock>();
}

//////////////////////////////////////////////////////////////////////
//
// Function
//
Function::Function(Factory* factory, FunctionType* type, int id)
    : Value(type), id_(id) {
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

Type* Function::parameters_type() const {
  return function_type()->parameters_type();
}

Type* Function::return_type() const {
  return function_type()->return_type();
}

// Literal
Literal::Literal(Type* type) : Value(type) {
}

// NullLiteral
NullLiteral::NullLiteral(Type* type) : Literal(type) {
}

// Reference
Reference::Reference(Type* type, AtomicString* name)
    : Literal(type), name_(name) {
}

//////////////////////////////////////////////////////////////////////
//
// UseDefNode
//
UseDefNode::UseDefNode() : instruction_(nullptr), value_(nullptr) {
}

void UseDefNode::Init(Instruction* instruction, Value* value) {
  DCHECK(instruction);
  DCHECK(value);
  DCHECK(!instruction_);
  DCHECK(!value_);
  DCHECK(value);
  instruction_ = instruction;
  value_ = value;
  value_->Use(this);
}

void UseDefNode::Reset() {
  DCHECK(value_) << "Already reset.";
  value_->Unuse(this);
  value_ = nullptr;
}

void UseDefNode::SetValue(Value* new_value) {
  DCHECK(new_value);
  DCHECK(value_);
  value_->Unuse(this);
  new_value->Use(this);
  value_ = new_value;
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

// VoidValue
VoidValue::VoidValue(VoidType* type, int data) : Literal(type) {
  DCHECK(!data);
}

}  // namespace hir
}  // namespace elang
