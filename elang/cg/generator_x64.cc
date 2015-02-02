// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/cg/generator.h"

#include "base/logging.h"
#include "elang/hir/instructions.h"
#include "elang/hir/instruction_visitor.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/isa.h"
#include "elang/lir/literals.h"

namespace elang {
namespace cg {

using lir::Isa;

//////////////////////////////////////////////////////////////////////
//
// Generator
//
lir::Value Generator::AllocateRegister(hir::Value* hir_value,
                                       int min_bit_size) {
  auto const primitive_type = hir_value->type()->as<hir::PrimitiveType>();
  if (!primitive_type)
    return factory()->NewRegister();
  if (primitive_type->is<hir::Float32Type>())
    return factory()->NewFloatRegister(lir::Value::Size::Size32);
  if (primitive_type->is<hir::Float64Type>())
    return factory()->NewFloatRegister(lir::Value::Size::Size64);
  switch (std::max(primitive_type->bit_size(), min_bit_size)) {
    case 1:
    case 8:
      return factory()->NewRegister(lir::Value::Size::Size8);
    case 16:
      return factory()->NewRegister(lir::Value::Size::Size16);
    case 32:
      return factory()->NewRegister(lir::Value::Size::Size32);
    case 64:
      return factory()->NewRegister(lir::Value::Size::Size64);
  }
  NOTREACHED() << "unsupported bit size: " << *primitive_type;
  return factory()->NewRegister();
}

void Generator::EmitSetLiteral(lir::Value output, hir::Literal* value) {
  DCHECK(output.is_register());

  if (auto const literal = value->as<hir::BoolLiteral>()) {
    Emit(factory()->NewLoadInstruction(
        output, factory()->NewIntValue(output.size, literal->data())));
    return;
  }
  if (auto const literal = value->as<hir::Int32Literal>()) {
    Emit(factory()->NewLoadInstruction(
        output, factory()->NewIntValue(output.size, literal->data())));
    return;
  }
  if (auto const literal = value->as<hir::Int64Literal>()) {
    Emit(factory()->NewLoadInstruction(
        output, factory()->NewIntValue(output.size, literal->data())));
    return;
  }
  NOTREACHED() << "unsupported hir::Literal: " << *value;
}

void Generator::EmitSetValue(lir::Value output, hir::Value* value) {
  DCHECK(output.is_register());
  if (auto const instr = value->as<hir::Instruction>()) {
    EmitCopy(output, register_map_[value]);
    return;
  }
  if (auto const literal = value->as<hir::Literal>()) {
    EmitSetLiteral(output, literal);
    return;
  }
  NOTREACHED() << "unsupported hir::Value: " << *value;
}

lir::Value Generator::MapRegister(hir::Value* value, int min_bit_size) {
  auto const it = register_map_.find(value);
  if (it != register_map_.end())
    return it->second;
  auto const new_register = AllocateRegister(value, min_bit_size);
  register_map_[value] = new_register;
  return new_register;
}

// hir::InstructionVisitor

// Load parameters from registers and stack
void Generator::VisitEntry(hir::EntryInstruction* instr) {
  auto const parameters_type = instr->output_type();
  if (parameters_type->is<hir::VoidType>())
    return;
  auto const tuple = parameters_type->as<hir::TupleType>();
  if (!tuple) {
    auto const output = MapRegister(instr, 32);
    auto const input = lir::Isa::GetParameterAt(output, 0);
    DCHECK(input.is_register());
    EmitCopy(output, input);
    return;
  }
  for (auto const user : instr->users()) {
    auto const get_instr = user->instruction()->as<hir::GetInstruction>();
    if (!get_instr)
      continue;
    auto const output = MapRegister(get_instr, 32);
    auto const input = lir::Isa::GetParameterAt(output, get_instr->index());
    if (input.is_register()) {
      EmitCopy(output, input);
      continue;
    }
    Emit(factory()->NewLoadInstruction(output, input));
  }
}

// Set return value and emit 'ret' instruction.
void Generator::VisitRet(hir::RetInstruction* instr) {
  auto const value = instr->input(0);
  if (!value->is<hir::VoidValue>()) {
    auto const primitive_type = value->type()->as<hir::PrimitiveType>();
    if (primitive_type->is_float())
      EmitSetValue(Isa::GetRegister(lir::isa::XMM0), value);
    else if (primitive_type->bit_size() <= 32)
      EmitSetValue(Isa::GetRegister(lir::isa::EAX), value);
    else
      EmitSetValue(Isa::GetRegister(lir::isa::RAX), value);
  }
  editor()->SetReturn();
}

}  // namespace cg
}  // namespace elang
