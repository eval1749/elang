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
#include "elang/lir/literals.h"
#include "elang/lir/target.h"

namespace elang {
namespace cg {

using lir::Target;

//////////////////////////////////////////////////////////////////////
//
// Generator
//

void Generator::EmitSetValue(lir::Value output, hir::Value* value) {
  DCHECK(output.is_register());
  auto const input = MapInput(value);
  if (input.is_register()) {
    EmitCopy(output, input);
    return;
  }
  Emit(NewLiteralInstruction(output, input));
}

lir::Value Generator::GenerateShl(lir::Value input, int shift_count) {
  DCHECK_GE(shift_count, 0);
  shift_count &= lir::Value::BitSizeOf(input) - 1;
  DCHECK_GE(shift_count, 0);
  if (!shift_count)
    return input;
  auto const output = NewRegister(input);
  if (shift_count == 1) {
    Emit(NewAddInstruction(output, input, input));
    return output;
  }
  Emit(NewShlInstruction(output, input, lir::Value::SmallInt32(shift_count)));
  return output;
}

lir::Value Generator::MapInput(hir::Value* value) {
  if (auto const instr = value->as<hir::Instruction>()) {
    auto const it = register_map_.find(instr);
    DCHECK(it != register_map_.end());
    return it->second;
  }

  if (auto const literal = value->as<hir::BoolLiteral>())
    return NewIntValue(lir::Value::Int8Type(), literal->data());
  if (auto const literal = value->as<hir::Float32Literal>())
    return NewFloat32Value(literal->data());
  if (auto const literal = value->as<hir::Float64Literal>())
    return NewFloat64Value(literal->data());
  if (auto const literal = value->as<hir::Int8Literal>())
    return NewIntValue(lir::Value::Int8Type(), literal->data());
  if (auto const literal = value->as<hir::Int16Literal>())
    return NewIntValue(lir::Value::Int16Type(), literal->data());
  if (auto const literal = value->as<hir::Int32Literal>())
    return NewIntValue(lir::Value::Int32Type(), literal->data());
  if (auto const literal = value->as<hir::Int64Literal>())
    return NewIntValue(lir::Value::Int64Type(), literal->data());
  if (auto const literal = value->as<hir::UInt8Literal>())
    return NewIntValue(lir::Value::Int8Type(), literal->data());
  if (auto const literal = value->as<hir::UInt16Literal>())
    return NewIntValue(lir::Value::Int16Type(), literal->data());
  if (auto const literal = value->as<hir::UInt32Literal>())
    return NewIntValue(lir::Value::Int32Type(), literal->data());
  if (auto const literal = value->as<hir::UInt64Literal>())
    return NewIntValue(lir::Value::Int64Type(), literal->data());

  if (auto const reference = value->as<hir::Reference>())
    return NewStringValue(reference->name());

  NOTREACHED() << "unsupported hir::Literal: " << *value;
  return NewIntValue(lir::Value::Int8Type(), 0);
}

// Get output register for instruction except for 'load'.
lir::Value Generator::MapOutput(hir::Instruction* instruction) {
  DCHECK(!register_map_.count(instruction));
  return MapRegister(instruction);
}

lir::Value Generator::MapRegister(hir::Value* value) {
  auto const it = register_map_.find(value);
  if (it != register_map_.end())
    return it->second;
  auto const new_register = NewRegister(MapType(value->type()));
  register_map_[value] = new_register;
  return new_register;
}

lir::Value Generator::MapType(hir::Type* type) {
  auto const primitive_type = type->as<hir::PrimitiveType>();
  if (!primitive_type)
    return lir::Value::Int64Type();
  if (primitive_type->is<hir::Float32Type>())
    return lir::Value::Float32Type();
  if (primitive_type->is<hir::Float64Type>())
    return lir::Value::Float64Type();
  switch (primitive_type->bit_size()) {
    case 1:
    case 8:
      return lir::Value::Int8Type();
    case 16:
      return lir::Value::Int16Type();
    case 32:
      return lir::Value::Int32Type();
    case 64:
      return lir::Value::Int64Type();
  }
  NOTREACHED() << "unsupported bit size: " << *primitive_type;
  return lir::Value::Float64Type();
}

// hir::InstructionVisitor

void Generator::VisitCall(hir::CallInstruction* instr) {
  auto const lir_callee = MapInput(instr->input(0));
  auto const argument = instr->input(1);

  if (argument->type()->is<hir::VoidType>()) {
    // No argument
    Emit(NewCallInstruction(lir_callee));
    return;
  }

  auto const arguments_instr = argument->as<hir::TupleInstruction>();
  if (!arguments_instr) {
    // One argument
    auto const lir_argument = MapInput(argument);
    EmitCopy(Target::GetArgumentAt(lir_argument, 0), lir_argument);
    Emit(NewCallInstruction(lir_callee));
    return;
  }

  // Multiple arguments
  std::vector<lir::Value> inputs(arguments_instr->CountInputs());
  inputs.resize(0);
  std::vector<lir::Value> outputs(inputs.size());
  outputs.resize(0);
  auto position = 0;
  for (auto const argument : arguments_instr->inputs()) {
    auto const lir_argument = MapInput(argument);
    inputs.push_back(lir_argument);
    outputs.push_back(Target::GetArgumentAt(lir_argument, position));
    ++position;
  }
  Emit(NewPCopyInstruction(outputs, inputs));
  Emit(NewCallInstruction(lir_callee));
}

void Generator::VisitElement(hir::ElementInstruction* instr) {
  auto const indexes = instr->input(1)->as<hir::TupleInstruction>();
  if (indexes) {
    // Multiple dimensions array:
    //   T* %ptr = element %array_ptr, %index...
    //   =>
    //   pcopy RCX, RDX, ... = %array_ptr%, %index...
    //   call `CalculateRowMajorIndex` //   for multiple dimension array
    //   copy %row_major_index, EAX
    //   sext %row_major_index64, %row_major_index
    //   add %element_ptr = %array_ptr, %row_major_index64
    //   aload %output = %array_ptr, %element_ptr, sizeof(ArrayHeader)
    // or
    //   astore %array_ptr, %element_ptr, sizeof(ArrayHeader), %new_value
    // TODO(eval1749) We need to have helper function to calculate row-major-
    // index from array type.
    NOTREACHED() << "NYI: multiple dimension array access";
    // Layout of multiple dimensions array object:
    //  +0 object header
    //  +8 dimension[0]
    //  +16 dimension[1]
    //  ...
    //  +8*(n+1) element[0]
  }

  // Vector (single dimension array)
  //   T* %ptr = element %array_ptr, %index
  //   =>
  //   add %element_start = %array_ptr, sizeof(ArrayHeader)
  //   shl %offset = %index, log2(sizeof(element_type))
  //   sext %offset64 = %offset
  //   add %element_ptr = %element_start, %offset64
  auto const array_pointer = MapInput(instr->input(0));
  auto const element_type =
      MapType(instr->type()->as<hir::PointerType>()->pointee());

  // Layout of vector object:
  //  +0 object header
  //  +8 length
  //  +16 element[0]
  auto const sizeof_array_header =
      lir::Value::SmallInt64(lir::Value::SizeOf(Target::IntPtrType()) * 2);
  auto const element_start = NewRegister(Target::IntPtrType());
  Emit(NewAddInstruction(element_start, array_pointer, sizeof_array_header));

  auto const shift_count = lir::Value::Log2Of(element_type);
  auto const offset = GenerateShl(MapInput(instr->input(1)), shift_count);
  auto const offset64 = NewRegister(Target::IntPtrType());
  Emit(NewSignExtendInstruction(offset64, offset));

  Emit(NewAddInstruction(MapOutput(instr), element_start, offset64));
}

// Load parameters from registers and stack
void Generator::VisitEntry(hir::EntryInstruction* instr) {
  auto const parameters_type = instr->output_type();
  if (parameters_type->is<hir::VoidType>())
    return;
  auto const tuple = parameters_type->as<hir::TupleType>();
  if (!tuple) {
    auto const output = MapRegister(instr);
    auto const input = lir::Target::GetParameterAt(output, 0);
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
    auto const output = MapRegister(get_instr);
    outputs.push_back(output);
    inputs.push_back(lir::Target::GetParameterAt(output, get_instr->index()));
  }
  Emit(NewPCopyInstruction(outputs, inputs));
}

void Generator::VisitLoad(hir::LoadInstruction* instr) {
  Emit(NewLoadInstruction(MapOutput(instr), MapInput(instr->input(0)),
                          MapInput(instr->input(1))));
}

// Set return value and emit 'ret' instruction.
void Generator::VisitRet(hir::RetInstruction* instr) {
  auto const value = instr->input(0);
  if (value->is<hir::VoidValue>()) {
    editor()->SetReturn();
    return;
  }
  auto const primitive_type = value->type()->as<hir::PrimitiveValueType>();
  if (!primitive_type) {
    EmitSetValue(Target::GetRegister(lir::isa::RAX), value);
    editor()->SetReturn();
    return;
  }

  if (primitive_type->is_float()) {
    if (primitive_type->bit_size() == 64)
      EmitSetValue(Target::GetRegister(lir::isa::XMM0D), value);
    else
      EmitSetValue(Target::GetRegister(lir::isa::XMM0S), value);
    editor()->SetReturn();
    return;
  }

  if (primitive_type->bit_size() == 64) {
    EmitSetValue(Target::GetRegister(lir::isa::RAX), value);
    editor()->SetReturn();
    return;
  }

  auto const output = Target::GetRegister(lir::isa::EAX);
  auto const input = MapInput(value);
  if (primitive_type->bit_size() == 32 || !input.is_register()) {
    EmitSetValue(output, value);
    editor()->SetReturn();
    return;
  }

  if (primitive_type->is_signed()) {
    Emit(NewSignExtendInstruction(output, input));
    editor()->SetReturn();
    return;
  }
  Emit(NewZeroExtendInstruction(output, input));
  editor()->SetReturn();
}

}  // namespace cg
}  // namespace elang
