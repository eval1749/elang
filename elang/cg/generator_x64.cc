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

void Generator::EmitSetValue(lir::Value output, hir::Value* value) {
  DCHECK(output.is_register());
  auto const input = MapInput(output, value);
  if (input.is_register()) {
    EmitCopy(output, input);
    return;
  }
  Emit(factory()->NewLiteralInstruction(output, input));
}

lir::Value Generator::MapInput(lir::Value output, hir::Value* value) {
  if (auto const instr = value->as<hir::Instruction>()) {
    auto const it = register_map_.find(instr);
    DCHECK(it != register_map_.end());
    return it->second;
  }

  if (auto const literal = value->as<hir::BoolLiteral>())
    return factory()->NewIntValue(output.size, literal->data());
  if (auto const literal = value->as<hir::Float32Literal>())
    return factory()->NewFloat32Value(literal->data());
  if (auto const literal = value->as<hir::Float64Literal>())
    return factory()->NewFloat64Value(literal->data());
  if (auto const literal = value->as<hir::Int8Literal>())
    return factory()->NewIntValue(output.size, literal->data());
  if (auto const literal = value->as<hir::Int16Literal>())
    return factory()->NewIntValue(output.size, literal->data());
  if (auto const literal = value->as<hir::Int32Literal>())
    return factory()->NewIntValue(output.size, literal->data());
  if (auto const literal = value->as<hir::Int64Literal>())
    return factory()->NewIntValue(output.size, literal->data());
  if (auto const literal = value->as<hir::UInt8Literal>())
    return factory()->NewIntValue(output.size, literal->data());
  if (auto const literal = value->as<hir::UInt16Literal>())
    return factory()->NewIntValue(output.size, literal->data());
  if (auto const literal = value->as<hir::UInt32Literal>())
    return factory()->NewIntValue(output.size, literal->data());
  if (auto const literal = value->as<hir::UInt64Literal>())
    return factory()->NewIntValue(output.size, literal->data());

  if (auto const reference = value->as<hir::Reference>())
    return factory()->NewStringValue(reference->name());

  NOTREACHED() << "unsupported hir::Literal: " << *value;
  return factory()->NewIntValue(output.size, 0);
}

// Get output register for instruction except for 'load'.
lir::Value Generator::MapOutput(hir::Instruction* instruction) {
  DCHECK(!instruction->is<hir::LoadInstruction>());
  DCHECK(!register_map_.count(instruction));
  return MapRegister(instruction, 32);
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

void Generator::VisitCall(hir::CallInstruction* instr) {
  auto const callee_align =
      lir::Value(lir::Value::Type::Integer, lir::Value::Size::Size64,
                 lir::Value::Kind::Void);
  auto const lir_callee = MapInput(callee_align, instr->input(0));
  auto const argument = instr->input(1);

  if (argument->type()->is<hir::VoidType>()) {
    // No argument
    Emit(factory()->NewCallInstruction(lir_callee));
    return;
  }

  auto const arg_align =
      lir::Value(lir::Value::Type::Integer, lir::Value::Size::Size32,
                 lir::Value::Kind::Void);
  auto const arguments_instr = argument->as<hir::TupleInstruction>();
  if (!arguments_instr) {
    // One argument
    auto const lir_argument = MapInput(arg_align, argument);
    EmitCopy(Isa::GetArgumentAt(lir_argument, 0), lir_argument);
    Emit(factory()->NewCallInstruction(lir_callee));
    return;
  }

  // Multiple arguments
  std::vector<lir::Value> inputs(arguments_instr->CountInputs());
  inputs.resize(0);
  std::vector<lir::Value> outputs(inputs.size());
  outputs.resize(0);
  auto position = 0;
  for (auto const argument : arguments_instr->inputs()) {
    auto const lir_argument = MapInput(arg_align, argument);
    inputs.push_back(lir_argument);
    outputs.push_back(Isa::GetArgumentAt(lir_argument, position));
    ++position;
  }
  Emit(factory()->NewPCopyInstruction(outputs, inputs));
  Emit(factory()->NewCallInstruction(lir_callee));
}

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
  std::vector<lir::Value> inputs(tuple->size());
  inputs.resize(0);
  std::vector<lir::Value> outputs(inputs.size());
  outputs.resize(0);
  for (auto const user : instr->users()) {
    auto const get_instr = user->instruction()->as<hir::GetInstruction>();
    if (!get_instr)
      continue;
    auto const output = MapRegister(get_instr, 32);
    outputs.push_back(output);
    inputs.push_back(lir::Isa::GetParameterAt(output, get_instr->index()));
  }
  Emit(factory()->NewPCopyInstruction(outputs, inputs));
}

// Set return value and emit 'ret' instruction.
void Generator::VisitRet(hir::RetInstruction* instr) {
  auto const value = instr->input(0);
  if (!value->is<hir::VoidValue>()) {
    auto const primitive_type = value->type()->as<hir::PrimitiveType>();
    if (primitive_type->is_float()) {
      if (primitive_type->bit_size() == 64)
        EmitSetValue(Isa::GetRegister(lir::isa::XMM0D), value);
      else
        EmitSetValue(Isa::GetRegister(lir::isa::XMM0S), value);
    } else if (primitive_type->bit_size() <= 32) {
      EmitSetValue(Isa::GetRegister(lir::isa::EAX), value);
    } else {
      EmitSetValue(Isa::GetRegister(lir::isa::RAX), value);
    }
  }
  editor()->SetReturn();
}

}  // namespace cg
}  // namespace elang
