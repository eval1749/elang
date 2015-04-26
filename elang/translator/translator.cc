// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/translator/translator.h"

#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/scheduler/schedule.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace translator {

namespace {
int SizeOfType(ir::Type* type) {
  if (type->is<ir::IntPtrType>())
    return 8;
  if (type->is<ir::UIntPtrType>())
    return 8;
  if (auto const primitive_type = type->as<ir::PrimitiveType>())
    return primitive_type->bit_size() / 8;
  if (auto const tuple_type = type->as<ir::TupleType>()) {
    auto size = 0;
    for (auto const component : tuple_type->components())
      size += SizeOfType(component);
    return size;
  }
  if (auto const array_type = type->as<ir::ArrayType>()) {
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
// Translator
//
Translator::Translator(lir::Factory* factory, const ir::Schedule* schedule)
    : FactoryUser(factory),
      editor_(
          new lir::Editor(factory, NewFunction(factory, schedule->function()))),
      schedule_(*schedule) {
}

Translator::~Translator() {
}

lir::Function* Translator::function() const {
  return editor_->function();
}

lir::BasicBlock* Translator::BlockOf(ir::Node* node) const {
  DCHECK(node->IsBlockStart() || node->IsBlockEnd());
  auto const it = block_map_.find(node);
  DCHECK(it != block_map_.end());
  return it->second;
}

void Translator::Emit(lir::Instruction* instruction) {
  editor()->Append(instruction);
}

void Translator::EmitCopy(lir::Value output, lir::Value input) {
  DCHECK_NE(output, input);
  Emit(NewCopyInstruction(output, input));
}

void Translator::EmitSetValue(lir::Value output, ir::Node* node) {
  DCHECK(output.is_register());
  auto const input = MapInput(node);
  if (input.is_register()) {
    EmitCopy(output, input);
    return;
  }
  Emit(NewLiteralInstruction(output, input));
}

lir::Value Translator::MapInput(ir::Node* node) {
  DCHECK(node->IsData());

  if (auto const reference = node->as<ir::ReferenceNode>())
    return NewStringValue(reference->name());

  if (auto const size = node->as<ir::SizeOfNode>()) {
    return NewIntValue(lir::Value::IntPtrType(),
                       SizeOfType(size->output_type()));
  }

  if (!node->IsLiteral())
    return MapRegister(node);

  if (auto const literal = node->as<ir::BoolNode>())
    return NewIntValue(lir::Value::Int8Type(), literal->data());
  if (auto const literal = node->as<ir::Float32Node>())
    return NewFloat32Value(literal->data());
  if (auto const literal = node->as<ir::Float64Node>())
    return NewFloat64Value(literal->data());
  if (auto const literal = node->as<ir::Int8Node>())
    return NewIntValue(lir::Value::Int8Type(), literal->data());
  if (auto const literal = node->as<ir::Int16Node>())
    return NewIntValue(lir::Value::Int16Type(), literal->data());
  if (auto const literal = node->as<ir::Int32Node>())
    return NewIntValue(lir::Value::Int32Type(), literal->data());
  if (auto const literal = node->as<ir::Int64Node>())
    return NewIntValue(lir::Value::Int64Type(), literal->data());
  if (auto const literal = node->as<ir::IntPtrNode>())
    return NewIntValue(lir::Value::Int64Type(), literal->data());
  if (auto const literal = node->as<ir::UInt8Node>())
    return NewIntValue(lir::Value::Int8Type(), literal->data());
  if (auto const literal = node->as<ir::UInt16Node>())
    return NewIntValue(lir::Value::Int16Type(), literal->data());
  if (auto const literal = node->as<ir::UInt32Node>())
    return NewIntValue(lir::Value::Int32Type(), literal->data());
  if (auto const literal = node->as<ir::UInt64Node>())
    return NewIntValue(lir::Value::Int64Type(), literal->data());
  if (auto const literal = node->as<ir::UIntPtrNode>())
    return NewIntValue(lir::Value::IntPtrType(), literal->data());

  NOTREACHED() << "unsupported ir::Literal: " << *node;
  return NewIntValue(lir::Value::Int8Type(), 0);
}

lir::Value Translator::MapOutput(ir::Node* node) {
  DCHECK(!node->IsLiteral()) << *node;
  DCHECK(node->IsData()) << *node;
  return MapRegister(node);
}

lir::Value Translator::MapRegister(ir::Node* node) {
  DCHECK(!node->IsLiteral()) << *node;
  DCHECK(node->IsData()) << *node;
  auto const it = register_map_.find(node);
  if (it != register_map_.end())
    return it->second;
  auto const new_register = NewRegister(MapType(node->output_type()));
  register_map_.insert(std::make_pair(node, new_register));
  return new_register;
}

lir::Value Translator::MapType(ir::Type* type) {
  auto const primitive_type = type->as<ir::PrimitiveType>();
  if (!primitive_type)
    return lir::Value::Int64Type();
  if (primitive_type->is<ir::Float32Type>())
    return lir::Value::Float32Type();
  if (primitive_type->is<ir::Float64Type>())
    return lir::Value::Float64Type();
  if (primitive_type->is<ir::IntPtrType>())
    return lir::Value::IntPtrType();
  if (primitive_type->is<ir::UIntPtrType>())
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

lir::Function* Translator::NewFunction(lir::Factory* factory,
                                       ir::Function* ir_function) {
  auto const parameters_type = ir_function->parameters_type();

  if (parameters_type->is<ir::VoidType>()) {
    // No parameters
    return factory->NewFunction({});
  }

  if (auto const tuple_type = parameters_type->as<ir::TupleType>()) {
    // Multiple parameters
    std::vector<lir::Value> parameters;
    auto position = 0;
    for (auto const component : tuple_type->components()) {
      auto const parameter_type = MapType(component);
      parameters.push_back(
          lir::Target::GetParameterAt(parameter_type, position));
      ++position;
    }
    return factory->NewFunction(parameters);
  }

  // Single parameter
  auto const parameter_type = MapType(parameters_type);
  auto const parameter = lir::Target::GetParameterAt(parameter_type, 0);
  return factory->NewFunction({parameter});
}

// The entry point
lir::Function* Translator::Run() {
  {
    auto block = static_cast<lir::BasicBlock*>(nullptr);
    for (auto const node : schedule_.nodes()) {
      if (node->IsBlockStart()) {
        DCHECK(!block);
        DCHECK(!block_map_.count(node));
        if (node->opcode() == ir::Opcode::Entry)
          block = editor()->entry_block();
        else if (node->use_edges().begin()->from()->opcode() ==
                 ir::Opcode::Exit)
          block = editor()->exit_block();
        else
          block = editor()->NewBasicBlock(editor()->exit_block());
        block_map_.insert(std::make_pair(node, block));
        continue;
      }
      DCHECK(block);
      if (!node->IsBlockEnd())
        continue;
      DCHECK(!block_map_.count(node));
      block_map_.insert(std::make_pair(node, block));
      block = nullptr;
    }
    DCHECK(!block);
  }

  for (auto const node : schedule_.nodes()) {
    if (node->IsBlockStart())
      editor()->Edit(BlockOf(node));
    node->Accept(this);
    if (node->IsBlockEnd())
      editor()->Commit();
  }

  DCHECK(editor()->Validate());
  return editor()->function();
}

// ir::NodeVisitor

// Literal nodes
#define V(Name, ...) \
  void Translator::Visit##Name(ir::Name##Node* node) { NOTREACHED() << *node; }
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)

// Simple node 1
void Translator::VisitDynamicCast(ir::DynamicCastNode* node) {
  NOTREACHED() << *node;
}

void Translator::VisitExit(ir::ExitNode* node) {
}

void Translator::VisitGetData(ir::GetDataNode* node) {
}

void Translator::VisitGetEffect(ir::GetEffectNode* node) {
}

void Translator::VisitGetTuple(ir::GetTupleNode* node) {
}

void Translator::VisitIfException(ir::IfExceptionNode* node) {
}

void Translator::VisitIfFalse(ir::IfFalseNode* node) {
}

void Translator::VisitIfSuccess(ir::IfSuccessNode* node) {
}

void Translator::VisitIfTrue(ir::IfTrueNode* node) {
}

void Translator::VisitJump(ir::JumpNode* node) {
  editor()->SetJump(BlockOf(node->use_edges().begin()->from()));
}

void Translator::VisitStaticCast(ir::StaticCastNode* node) {
  // TODO(eval1749): NYI translate StaticCast
  NOTREACHED() << *node;
}

void Translator::VisitUnreachable(ir::UnreachableNode* node) {
  // TODO(eval1749): NYI translate Unreachable
  NOTREACHED() << *node;
}

// Simple node 2
void Translator::VisitElement(ir::ElementNode* node) {
  // TODO(eval1749): NYI translate Element
  NOTREACHED() << *node;
}

void Translator::VisitIf(ir::IfNode* node) {
  // TODO(eval1749): NYI translate If
  NOTREACHED() << *node;
}

void Translator::VisitIntShl(ir::IntShlNode* node) {
  // TODO(eval1749): NYI translate IntShl
  NOTREACHED() << *node;
}

void Translator::VisitIntShr(ir::IntShrNode* node) {
  // TODO(eval1749): NYI translate IntShr
  NOTREACHED() << *node;
}

void Translator::VisitLength(ir::LengthNode* node) {
  // TODO(eval1749): NYI translate Length
  NOTREACHED() << *node;
}

void Translator::VisitStackAlloc(ir::StackAllocNode* node) {
  // TODO(eval1749): NYI translate StackAlloc
  NOTREACHED() << *node;
}

void Translator::VisitSwitch(ir::SwitchNode* node) {
  // TODO(eval1749): NYI translate Switch
  NOTREACHED() << *node;
}

void Translator::VisitThrow(ir::ThrowNode* node) {
  // TODO(eval1749): NYI translate Throw
  NOTREACHED() << *node;
}

// Arithmetic node
void Translator::VisitFloatAdd(ir::FloatAddNode* node) {
  // TODO(eval1749): NYI translate FloatAdd
  NOTREACHED() << *node;
}

void Translator::VisitFloatDiv(ir::FloatDivNode* node) {
  // TODO(eval1749): NYI translate FloatDiv
  NOTREACHED() << *node;
}

void Translator::VisitFloatMod(ir::FloatModNode* node) {
  // TODO(eval1749): NYI translate FloatMod
  NOTREACHED() << *node;
}

void Translator::VisitFloatMul(ir::FloatMulNode* node) {
  // TODO(eval1749): NYI translate FloatMul
  NOTREACHED() << *node;
}

void Translator::VisitFloatSub(ir::FloatSubNode* node) {
  // TODO(eval1749): NYI translate FloatSub
  NOTREACHED() << *node;
}

void Translator::VisitIntBitAnd(ir::IntBitAndNode* node) {
  // TODO(eval1749): NYI translate IntBitAnd
  NOTREACHED() << *node;
}

void Translator::VisitIntBitOr(ir::IntBitOrNode* node) {
  // TODO(eval1749): NYI translate IntBitOr
  NOTREACHED() << *node;
}

void Translator::VisitIntBitXor(ir::IntBitXorNode* node) {
  // TODO(eval1749): NYI translate IntBitXor
  NOTREACHED() << *node;
}

void Translator::VisitIntAdd(ir::IntAddNode* node) {
  // TODO(eval1749): NYI translate IntAdd
  NOTREACHED() << *node;
}

void Translator::VisitIntDiv(ir::IntDivNode* node) {
  // TODO(eval1749): NYI translate IntDiv
  NOTREACHED() << *node;
}

void Translator::VisitIntMod(ir::IntModNode* node) {
  // TODO(eval1749): NYI translate IntMod
  NOTREACHED() << *node;
}

void Translator::VisitIntMul(ir::IntMulNode* node) {
  // TODO(eval1749): NYI translate IntMul
  NOTREACHED() << *node;
}

void Translator::VisitIntSub(ir::IntSubNode* node) {
  // TODO(eval1749): NYI translate IntSub
  NOTREACHED() << *node;
}

// Simple nodes with three inputs
void Translator::VisitLoad(ir::LoadNode* node) {
  // TODO(eval1749): NYI translate Load
  NOTREACHED() << *node;
}

// Simple node 4 inputs
void Translator::VisitCall(ir::CallNode* node) {
  // TODO(eval1749): NYI translate Call
  NOTREACHED() << *node;
}

// Variadic inputs node
void Translator::VisitCase(ir::CaseNode* node) {
  // TODO(eval1749): NYI translate Case
  NOTREACHED() << *node;
}

void Translator::VisitTuple(ir::TupleNode* node) {
  // TODO(eval1749): NYI translate Tuple
  NOTREACHED() << *node;
}

// Non simple inputs node
void Translator::VisitEffectPhi(ir::EffectPhiNode* node) {
  // TODO(eval1749): NYI translate EffectPhi
  NOTREACHED() << *node;
}

void Translator::VisitEntry(ir::EntryNode* node) {
  // nothing to do
}

void Translator::VisitFloatCmp(ir::FloatCmpNode* node) {
  // TODO(eval1749): NYI translate FloatCmp
  NOTREACHED() << *node;
}

void Translator::VisitFunctionReference(ir::FunctionReferenceNode* node) {
  // TODO(eval1749): NYI translate FunctionReference
  NOTREACHED() << *node;
}

void Translator::VisitGet(ir::GetNode* node) {
  // TODO(eval1749): NYI translate Get
  NOTREACHED() << *node;
}

void Translator::VisitIntCmp(ir::IntCmpNode* node) {
  // TODO(eval1749): NYI translate IntCmp
  NOTREACHED() << *node;
}

void Translator::VisitLoop(ir::LoopNode* node) {
  // nothing to do
}

void Translator::VisitMerge(ir::MergeNode* node) {
  // nothing to do
}

void Translator::VisitNull(ir::NullNode* node) {
  // nothing to do
}

void Translator::VisitParameter(ir::ParameterNode* node) {
  // TODO(eval1749): NYI translate Parameter
  NOTREACHED() << *node;
}

void Translator::VisitPhi(ir::PhiNode* node) {
  // TODO(eval1749): NYI translate Phi
  NOTREACHED() << *node;
}

void Translator::VisitReference(ir::ReferenceNode* node) {
  // TODO(eval1749): NYI translate Reference
  NOTREACHED() << *node;
}

void Translator::VisitSizeOf(ir::SizeOfNode* node) {
  // nothing to do
}

void Translator::VisitStore(ir::StoreNode* node) {
  // TODO(eval1749): NYI translate Store
  NOTREACHED() << *node;
}

void Translator::VisitVoid(ir::VoidNode* node) {
  // nothing to do
}

}  // namespace translator
}  // namespace elang
