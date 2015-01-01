// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"

namespace elang {
namespace hir {

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
// Instruction
//
bool Instruction::CanBeRemoved() const {
  return !IsTerminator() && users().empty();
}

bool Instruction::IsExit() const {
  return false;
}

bool Instruction::IsTerminator() const {
  return false;
}

Instruction::Instruction(Type* output_type) : Operand(output_type) {
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
CallInstruction::CallInstruction(Factory* factory,
                                 Type* output_type,
                                 Operand* callee,
                                 Operand* arguments)
    : InstructionTemplate(output_type, callee, arguments) {
  __assume(factory);
}

bool CallInstruction::CanBeRemoved() const {
  // TODO(eval1749) We should return true for known side effect free functions.
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
ReturnInstruction::ReturnInstruction(Factory* factory,
                                     Type* output_type,
                                     Operand* value)
    : InstructionTemplate(output_type, value) {
}

ReturnInstruction* ReturnInstruction::New(Factory* factory, Operand* value) {
  return InstructionTemplate::New(factory, factory->types()->GetVoidType(),
                                  value);
}

bool ReturnInstruction::IsExit() const {
  return true;
}

bool ReturnInstruction::IsTerminator() const {
  return true;
}

}  // namespace hir
}  // namespace elang
