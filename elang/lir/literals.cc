// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_visitor.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

#define V(Name, ...) \
  void Name::Accept(LiteralVisitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_LIR_LITERAL(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Literal
//
Literal::Literal() {
}

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
BasicBlock::BasicBlock(Value value)
    : function_(nullptr), id_(0), value_(value) {
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

//////////////////////////////////////////////////////////////////////
//
// Function
//
Function::Function(Value value) : value_(value) {
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

int Function::id() const {
  return value_.data;
}

//////////////////////////////////////////////////////////////////////
//
// Simple Literals
//
#define V(Name, name, c_type, ...) \
  Name##Literal::Name##Literal(c_type data) : data_(data) {}
FOR_EACH_LIR_SIMPLE_LITERAL(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// StringLiteral
//
StringLiteral::StringLiteral(base::StringPiece16 data) : data_(data) {
}

}  // namespace lir
}  // namespace elang
