// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/basic_block_editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/function_editor.h"
#include "elang/hir/instructions.h"
#include "elang/hir/operands.h"
#include "elang/hir/operand_visitor.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {

#define V(Name, ...) \
  void Name::Accept(OperandVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_HIR_OPERAND(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// UseDefNode
//
UseDefNode::UseDefNode() : instruction_(nullptr), operand_(nullptr) {
}

void UseDefNode::Init(Instruction* instruction, Operand* operand) {
  DCHECK(!instruction_);
  DCHECK(!operand_);
  DCHECK(operand);
  instruction_ = instruction;
  operand_ = operand;
  operand_->Use(this);
}

void UseDefNode::SetOperand(Operand* new_operand) {
  if (operand_)
    operand_->Unuse(this);
  new_operand->Use(this);
}

//////////////////////////////////////////////////////////////////////
//
// Operand
//
Operand::Operand(Type* type) : type_(type) {
}

void Operand::Accept(OperandVisitor* visitor) {
  __assume(visitor);
  NOTREACHED();
}

void Operand::Use(UseDefNode* operand_holder) {
  use_def_list_.AppendNode(operand_holder);
}

void Operand::Unuse(UseDefNode* operand_holder) {
  use_def_list_.RemoveNode(operand_holder);
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
FOR_EACH_HIR_LITERAL_OPERAND(V)
#undef V

Literal::Literal(Type* type) : Operand(type) {
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
    : Operand(factory->GetVoidType()),
      function_(nullptr),
      id_(0),
      last_instruction_id_(0) {
}

Instruction* BasicBlock::first_instruction() const {
  return instructions_.first_node();
}

Instruction* BasicBlock::last_instruction() const {
  return instructions_.last_node();
}

void BasicBlock::set_id(int new_id) {
  DCHECK_GT(new_id, 0);
  DCHECK(!id_);
  id_ = new_id;
}

//////////////////////////////////////////////////////////////////////
//
// Function
//
Function::Function(Factory* factory, FunctionType* type)
    : Operand(type), function_(nullptr), last_basic_block_id_(0) {
  FunctionEditor function(factory, this);
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

}  // namespace hir
}  // namespace elang
