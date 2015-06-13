// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/cg/generator.h"

#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/values.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"

namespace elang {
namespace cg {

namespace {
int SizeOfType(hir::Type* type) {
  if (type->is<hir::IntPtrType>())
    return 8;
  if (type->is<hir::UIntPtrType>())
    return 8;
  if (auto const primitive_type = type->as<hir::PrimitiveType>())
    return primitive_type->bit_size() / 8;
  if (auto const tuple_type = type->as<hir::TupleType>()) {
    auto size = 0;
    for (auto const member : tuple_type->members())
      size += SizeOfType(member);
    return size;
  }
  if (auto const array_type = type->as<hir::ArrayType>()) {
    auto size = SizeOfType(array_type->element_type());
    for (auto const dimension : array_type->dimensions()) {
      DCHECK_GE(dimension, 0);
      size *= dimension;
    }
    return size;
  }
  return 8;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Generator
//
Generator::Generator(lir::Factory* factory, hir::Function* hir_function)
    : FactoryUser(factory),
      editor_(new lir::Editor(factory, NewFunction(factory, hir_function))),
      hir_function_(hir_function) {
  block_map_[hir_function->entry_block()] = function()->entry_block();
  block_map_[hir_function->exit_block()] = function()->exit_block();
}

Generator::~Generator() {
}

lir::Function* Generator::function() const {
  return editor_->function();
}

void Generator::Emit(lir::Instruction* instruction) {
  editor()->Append(instruction);
}

void Generator::EmitCopy(lir::Value output, lir::Value input) {
  DCHECK_NE(output, input);
  Emit(NewCopyInstruction(output, input));
}

void Generator::EmitSetValue(lir::Value output, hir::Value* value) {
  DCHECK(output.is_register());
  auto const input = MapInput(value);
  if (input.is_register()) {
    EmitCopy(output, input);
    return;
  }
  Emit(NewLiteralInstruction(output, input));
}

lir::Function* Generator::Generate() {
  for (auto const hir_block : hir_function_->basic_blocks()) {
    editor()->Edit(MapBlock(hir_block));
    for (auto const phi : hir_block->phi_instructions())
      editor()->NewPhi(MapOutput(phi));
    for (auto const instr : hir_block->instructions())
      const_cast<hir::Instruction*>(instr)->Accept(this);
    editor()->Commit();
  }

  // Set phi operands.
  for (auto const hir_block : hir_function_->basic_blocks()) {
    DCHECK(block_map_.count(hir_block));
    auto const block = block_map_[hir_block];
    editor()->Edit(block);
    auto phis = block->phi_instructions().begin();
    for (auto const hir_phi : hir_block->phi_instructions()) {
      for (auto const hir_phi_input : hir_phi->phi_inputs()) {
        editor()->SetPhiInput(*phis, MapBlock(hir_phi_input->basic_block()),
                              MapInput(hir_phi_input->value()));
      }
      ++phis;
    }
    editor()->Commit();
  }
  return editor()->function();
}

void Generator::HandleComparison(hir::Instruction* instr,
                                 lir::IntCondition signed_condition,
                                 lir::IntCondition unsigned_condition,
                                 lir::FloatCondition float_condition) {
  auto const output = NewConditional();
  DCHECK(!register_map_.count(instr));
  register_map_[instr] = output;

  auto const left = MapInput(instr->input(0));
  auto const right = MapInput(instr->input(1));
  auto const primitive_type =
      instr->input(0)->type()->as<hir::PrimitiveValueType>();
  if (!primitive_type) {
    DCHECK_EQ(lir::IntCondition::Equal, signed_condition);
    Emit(NewCmpInstruction(output, signed_condition, left, right));
    return;
  }

  if (primitive_type->is_float()) {
    Emit(NewFloatCmpInstruction(output, float_condition, left, right));
    return;
  }

  if (primitive_type->is_signed()) {
    Emit(NewCmpInstruction(output, signed_condition, left, right));
    return;
  }

  Emit(NewCmpInstruction(output, unsigned_condition, left, right));
}

lir::BasicBlock* Generator::MapBlock(hir::BasicBlock* hir_block) {
  auto const it = block_map_.find(hir_block);
  if (it != block_map_.end())
    return it->second;
  auto const block = editor()->NewBasicBlock(editor()->exit_block());
  block_map_[hir_block] = block;
  return block;
}

lir::Value Generator::MapInput(hir::Value* value) {
  // TODO(eval1749) We should process |value| in reverse post order to ensure
  // |value| is mapped before it used.
  if (auto const instr = value->as<hir::Instruction>())
    return MapRegister(instr);

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
  if (auto const literal = value->as<hir::IntPtrLiteral>())
    return NewIntValue(lir::Value::Int64Type(), literal->data());
  if (auto const literal = value->as<hir::UInt8Literal>())
    return NewIntValue(lir::Value::Int8Type(), literal->data());
  if (auto const literal = value->as<hir::UInt16Literal>())
    return NewIntValue(lir::Value::Int16Type(), literal->data());
  if (auto const literal = value->as<hir::UInt32Literal>())
    return NewIntValue(lir::Value::Int32Type(), literal->data());
  if (auto const literal = value->as<hir::UInt64Literal>())
    return NewIntValue(lir::Value::Int64Type(), literal->data());
  if (auto const literal = value->as<hir::UIntPtrLiteral>())
    return NewIntValue(lir::Value::IntPtrType(), literal->data());

  if (auto const reference = value->as<hir::Reference>())
    return NewStringValue(reference->name());

  if (auto const size = value->as<hir::SizeOf>())
    return NewIntValue(lir::Value::Int64Type(), SizeOfType(value->type()));

  NOTREACHED() << "unsupported hir::Literal: " << *value;
  return NewIntValue(lir::Value::Int8Type(), 0);
}

// Get output register for instruction except for 'load'.
lir::Value Generator::MapOutput(hir::Instruction* instr) {
  // TODO(eval1749) We should process |value| in reverse post order to ensure
  // |value| is mapped before it used.
  if (!instr->is<hir::PhiInstruction>())
    DCHECK(!register_map_.count(instr)) << *instr;
  return MapRegister(instr);
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
  if (primitive_type->is<hir::IntPtrType>())
    return lir::Value::IntPtrType();
  if (primitive_type->is<hir::UIntPtrType>())
    return lir::Value::IntPtrType();
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

lir::Function* Generator::NewFunction(lir::Factory* factory,
                                      hir::Function* hir_function) {
  auto const parameters_type = hir_function->parameters_type();

  if (parameters_type->is<hir::VoidType>()) {
    // No parameters
    return factory->NewFunction({});
  }

  if (auto const tuple_type = parameters_type->as<hir::TupleType>()) {
    // Multiple parameters
    std::vector<lir::Value> parameters;
    auto position = 0;
    for (auto const hir_type : tuple_type->members()) {
      auto const parameter_type = MapType(hir_type);
      parameters.push_back(lir::Target::ParameterAt(parameter_type, position));
      ++position;
    }
    return factory->NewFunction(parameters);
  }

  // Single parameter
  auto const parameter_type = MapType(parameters_type);
  auto const parameter = lir::Target::ParameterAt(parameter_type, 0);
  return factory->NewFunction({parameter});
}

// hir::InstructionVisitor

void Generator::DoDefaultVisit(hir::Instruction* instr) {
  NOTREACHED() << *instr;
}

void Generator::VisitBranch(hir::BranchInstruction* instr) {
  editor()->SetBranch(MapInput(instr->input(0)),
                      MapBlock(instr->input(1)->as<hir::BasicBlock>()),
                      MapBlock(instr->input(2)->as<hir::BasicBlock>()));
}

void Generator::VisitEq(hir::EqInstruction* instr) {
  HandleComparison(instr, lir::IntCondition::Equal, lir::IntCondition::Equal,
                   lir::FloatCondition::OrderedEqual);
}

void Generator::VisitExit(hir::ExitInstruction* instr) {
  DCHECK(instr);
}

void Generator::VisitGe(hir::GeInstruction* instr) {
  HandleComparison(instr, lir::IntCondition::SignedGreaterThanOrEqual,
                   lir::IntCondition::UnsignedGreaterThanOrEqual,
                   lir::FloatCondition::OrderedGreaterThanOrEqual);
}

void Generator::VisitGet(hir::GetInstruction* instr) {
  DCHECK(instr);
}

void Generator::VisitGt(hir::GtInstruction* instr) {
  HandleComparison(instr, lir::IntCondition::SignedGreaterThan,
                   lir::IntCondition::UnsignedGreaterThan,
                   lir::FloatCondition::OrderedGreaterThan);
}

void Generator::VisitJump(hir::JumpInstruction* instr) {
  editor()->SetJump(MapBlock(instr->input(0)->as<hir::BasicBlock>()));
}

void Generator::VisitLe(hir::LeInstruction* instr) {
  HandleComparison(instr, lir::IntCondition::SignedLessThanOrEqual,
                   lir::IntCondition::UnsignedLessThanOrEqual,
                   lir::FloatCondition::OrderedLessThanOrEqual);
}

void Generator::VisitLoad(hir::LoadInstruction* instr) {
  Emit(NewLoadInstruction(MapOutput(instr), MapInput(instr->input(0)),
                          MapInput(instr->input(1)),
                          lir::Value::SmallInt32(0)));
}

void Generator::VisitLt(hir::LtInstruction* instr) {
  HandleComparison(instr, lir::IntCondition::SignedLessThan,
                   lir::IntCondition::UnsignedLessThan,
                   lir::FloatCondition::OrderedLessThan);
}

void Generator::VisitNe(hir::NeInstruction* instr) {
  HandleComparison(instr, lir::IntCondition::NotEqual,
                   lir::IntCondition::NotEqual,
                   lir::FloatCondition::OrderedNotEqual);
}

void Generator::VisitStaticCast(hir::StaticCastInstruction* instr) {
  DCHECK(!register_map_.count(instr));
  register_map_[instr] = MapInput(instr->input(0));
}

void Generator::VisitTuple(hir::TupleInstruction* instr) {
  DCHECK(instr);
}

#define V(Name, ...)                                               \
  void Generator::Visit##Name(hir::Name##Instruction* instr) {     \
    auto const output = MapOutput(instr);                          \
    Emit(New##Name##Instruction(output, MapInput(instr->input(0)), \
                                MapInput(instr->input(1))));       \
  }
#define NewDivInstruction NewIntDivInstruction
#define NewModInstruction NewIntModInstruction
#define NewMulInstruction NewIntMulInstruction
#define NewSubInstruction NewIntSubInstruction
FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
FOR_EACH_BITWISE_BINARY_OPERATION(V)
FOR_EACH_BITWISE_SHIFT_OPERATION(V)
#undef V

}  // namespace cg
}  // namespace elang
