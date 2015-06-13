// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <utility>
#include <vector>

#include "elang/translator/translator.h"

#include "elang/lir/editor.h"
#include "elang/lir/error_data.h"
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

lir::IntCondition MapCondition(ir::IntCondition condition) {
#define V(Name, ...)                                                \
  DCHECK_EQ(static_cast<ir::IntCondition>(lir::IntCondition::Name), \
            ir::IntCondition::Name);
  FOR_EACH_OPTIMIZER_INTEGER_CONDITION(V)
#undef V
  return static_cast<lir::IntCondition>(condition);
}

lir::Value PromoteType(lir::Value type) {
  return type.is_int8() || type.is_int16() ? lir::Value::Int32Type() : type;
}

ir::Node* SelectNode(const ir::Node* node, ir::Opcode opcode) {
  for (auto const edge : node->use_edges()) {
    if (edge->from()->opcode() == opcode)
      return edge->from();
  }
  NOTREACHED() << opcode << " " << *node;
  return nullptr;
}

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
  DCHECK(node->IsBlockStart() || node->IsBlockEnd()) << *node;
  auto const it = block_map_.find(node);
  DCHECK(it != block_map_.end()) << *node;
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
  DCHECK(output.is_register()) << output;
  auto const input = MapInput(node);
  if (input.is_register()) {
    EmitCopy(output, input);
    return;
  }
  Emit(NewLiteralInstruction(output, input));
}

lir::Value Translator::EmitShl(lir::Value input, int shift_count) {
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

lir::Value Translator::MapInput(ir::Node* node) {
  DCHECK(node->IsData()) << *node;

  if (auto const reference = node->as<ir::ReferenceNode>())
    return NewStringValue(reference->name());

  if (auto const size = node->as<ir::SizeOfNode>()) {
    return NewIntValue(lir::Value::IntPtrType(),
                       SizeOfType(size->output_type()));
  }

  if (!node->IsLiteral()) {
    auto const it = register_map_.find(node);
    DCHECK(it != register_map_.end()) << *node;
    return it->second;
  }

  auto const value = MapLiteral(node);
  if (value.is_int8() || value.is_int16())
    return NewIntValue(lir::Value::Int32Type(), value.data);
  return value;
}

lir::Value Translator::MapLiteral(ir::Node* node) {
  DCHECK(node->IsLiteral()) << *node;
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
  auto const it = register_map_.find(node);
  DCHECK(it == register_map_.end()) << *node << ": " << it->second;
  auto const type = PromoteType(MapType(node->output_type()));
  auto const new_register = NewRegister(type);
  register_map_.insert(std::make_pair(node, new_register));
  return new_register;
}

lir::Value Translator::MapType(ir::Type* type) {
  if (type->is<ir::VoidType>())
    return lir::Value::VoidType();
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

void Translator::PopulatePhiOperands() {
  for (auto const node : schedule_.nodes()) {
    auto const phi_owner = node->as<ir::PhiOwnerNode>();
    if (!phi_owner)
      continue;
    auto const block = BlockOf(phi_owner);
    editor()->Edit(block);
    std::unordered_map<lir::Value, lir::PhiInstruction*> phi_map;
    for (auto const phi : block->phi_instructions())
      phi_map.insert(std::make_pair(phi->output(0), phi));
    for (auto const phi_node : phi_owner->phi_nodes()) {
      if (!phi_node->IsUsed())
        continue;
      // Note: We've already map |phi_node| as output register.
      auto const it = phi_map.find(MapInput(phi_node));
      DCHECK(it != phi_map.end()) << *phi_node;
      auto const phi = it->second;
      for (auto const phi_input : phi_node->phi_inputs()) {
        editor()->SetPhiInput(phi, BlockOf(phi_input->control()),
                              MapInput(phi_input->value()));
      }
    }
    editor()->Commit();
  }
}

void Translator::PrepareBlocks() {
  auto block = static_cast<lir::BasicBlock*>(nullptr);
  auto const exit_block = editor()->exit_block();
  auto start_node = static_cast<ir::Node*>(nullptr);
  auto last_node = start_node;
  for (auto const node : schedule_.nodes()) {
    if (node->IsBlockStart()) {
      DCHECK(!start_node) << *node;
      DCHECK(!block) << *node;
      DCHECK(!block_map_.count(node)) << *node;
      if (node->opcode() == ir::Opcode::Entry)
        block = editor()->entry_block();
      else if (node->use_edges().begin()->from()->opcode() == ir::Opcode::Exit)
        block = exit_block;
      else
        block = editor()->NewBasicBlock(exit_block);
      start_node = node;
      block_map_.insert(std::make_pair(start_node, block));
      last_node = node;
      continue;
    }
    DCHECK(start_node) << *node;
    DCHECK(block) << *node;
    if (node->IsBlockEnd()) {
      block_map_.insert(std::make_pair(node, block));
      block = nullptr;
      start_node = nullptr;
      last_node = node;
      continue;
    }
    last_node = node;
  }
  DCHECK_EQ(ir::Opcode::Exit, last_node->opcode()) << *last_node;
  DCHECK(!block) << block;
}

// The entry point
lir::Function* Translator::Run() {
  PrepareBlocks();

  for (auto const node : schedule_.nodes()) {
    if (node->IsBlockStart()) {
      editor()->Edit(BlockOf(node));
      node->Accept(this);
      continue;
    }

    if (node->IsBlockEnd()) {
      node->Accept(this);
      editor()->Commit();
      continue;
    }

    node->Accept(this);
  }

  PopulatePhiOperands();

  DCHECK(editor()->Validate()) << *editor();
  return editor()->function();
}

lir::Value Translator::TranslateConditional(ir::Node* node) {
  if (node->opcode() == ir::Opcode::IntCmp ||
      node->opcode() == ir::Opcode::FloatCmp) {
    return MapInput(node);
  }
  DCHECK(node->output_type()->is<ir::BoolType>()) << *node;
  auto const output = NewConditional();
  Emit(NewCmpInstruction(output, lir::IntCondition::NotEqual, MapInput(node),
                         lir::Value::SmallInt32(0)));
  return output;
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
  // nothing to do
}

void Translator::VisitGetData(ir::GetDataNode* node) {
  DCHECK_EQ(ir::Opcode::Call, node->input(0)->opcode()) << *node << " "
                                                        << *node->input(0);
  auto const output = MapOutput(node);
  auto const return_type = PromoteType(output);
  auto const return_value = lir::Target::ReturnAt(return_type, 0);
  DCHECK_LE(lir::Value::SizeOf(output), lir::Value::SizeOf(return_type))
      << output << " " << return_type;
  if (output.size == return_type.size) {
    EmitCopy(output, return_value);
    return;
  }
  Emit(NewTruncateInstruction(output, return_value));
}

void Translator::VisitGetEffect(ir::GetEffectNode* node) {
  // nothing to do
}

void Translator::VisitGetTuple(ir::GetTupleNode* node) {
  DCHECK_EQ(ir::Opcode::Call, node->input(0)->opcode()) << *node << " "
                                                        << *node->input(0);
  // TODO(eval1749): NYI translate |GetTupleNode|
  NOTREACHED() << *node;
}

void Translator::VisitIfException(ir::IfExceptionNode* node) {
  // nothing to do
}

void Translator::VisitIfFalse(ir::IfFalseNode* node) {
  // nothing to do
}

void Translator::VisitIfSuccess(ir::IfSuccessNode* node) {
  // nothing to do
}

void Translator::VisitIfTrue(ir::IfTrueNode* node) {
  // nothing to do
}

void Translator::VisitJump(ir::JumpNode* node) {
  editor()->SetJump(BlockOf(node->SelectUserIfOne()));
}

void Translator::VisitStaticCast(ir::StaticCastNode* node) {
  auto const output = MapOutput(node);
  auto const input = MapInput(node->input(0));
  auto const input_type = node->input(0)->output_type();

  if (output.is_float()) {
    if (input.is_float()) {
      if (lir::Value::Log2Of(output) > lir::Value::Log2Of(input))
        return Emit(NewExtendInstruction(output, input));
      return Emit(NewTruncateInstruction(output, input));
    }
    if (input_type->is_signed())
      return Emit(NewSignedConvertInstruction(output, input));
    return Emit(NewUnsignedConvertInstruction(output, input));
  }

  if (input.is_float()) {
    if (node->output_type()->is_signed())
      return Emit(NewSignedConvertInstruction(output, input));
    return Emit(NewUnsignedConvertInstruction(output, input));
  }

  if (lir::Value::Log2Of(output) == lir::Value::Log2Of(input)) {
    register_map_[node] = input;
    return;
  }

  if (lir::Value::Log2Of(output) > lir::Value::Log2Of(input)) {
    if (input_type->is_signed())
      return Emit(NewSignExtendInstruction(output, input));
    return Emit(NewZeroExtendInstruction(output, input));
  }
  Emit(NewTruncateInstruction(output, input));
}

void Translator::VisitUnreachable(ir::UnreachableNode* node) {
  // TODO(eval1749): NYI translate Unreachable
  NOTREACHED() << *node;
}

// Simple node 2
void Translator::VisitElement(ir::ElementNode* node) {
  auto const indexes = node->input(1)->as<ir::TupleNode>();
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
    NOTREACHED() << "NYI: multiple dimension array access" << *node;
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
  auto const array_pointer = MapInput(node->input(0));
  auto const element_type =
      MapType(node->output_type()->as<ir::PointerType>()->pointee());

  // Layout of vector object:
  //  +0 object header
  //  +8 length
  //  +16 element[0]
  auto const sizeof_array_header =
      lir::Value::SmallInt64(lir::Value::SizeOf(lir::Value::IntPtrType()) * 2);
  auto const element_start = NewRegister(lir::Value::IntPtrType());
  Emit(NewAddInstruction(element_start, array_pointer, sizeof_array_header));

  auto const shift_count = lir::Value::Log2Of(element_type) - 3;
  auto const offset = EmitShl(MapInput(node->input(1)), shift_count);
  auto const offset64 = NewRegister(lir::Value::IntPtrType());
  Emit(NewSignExtendInstruction(offset64, offset));

  Emit(NewAddInstruction(MapOutput(node), element_start, offset64));
}

void Translator::VisitField(ir::FieldNode* node) {
}

void Translator::VisitIf(ir::IfNode* node) {
  auto const true_node = SelectNode(node, ir::Opcode::IfTrue);
  auto const false_node = SelectNode(node, ir::Opcode::IfFalse);
  editor()->SetBranch(TranslateConditional(node->input(1)), BlockOf(true_node),
                      BlockOf(false_node));
}

void Translator::VisitIntShr(ir::IntShrNode* node) {
  // TODO(eval1749): NYI translate IntShr
  NOTREACHED() << *node;
}

void Translator::VisitLength(ir::LengthNode* node) {
  // Layout of vector object:
  //  +0 object header
  //  +8 length[0]
  //  +12 length[1]
  //  ...
  //  +8+(rank-1)*4 length[rank-1]
  //  +8+rank*4 padding for align(16)
  //  +8+rank*4+align(16) element[0]
  //
  //  length int32 %length = %array, index
  //  =>
  //  add %length_ptr =
  //  load length = %array_ptr, %array_ptr,
  //                sizeof(ArrayHeader) + sizeof(int32) * index
  auto const pointer = NewRegister(lir::Value::IntPtrType());
  auto const rank = node->input(1)->as<ir::Int32Node>()->data();
  auto const offset = lir::Value::SizeOf(lir::Value::IntPtrType()) + rank * 4;
  auto const array_ptr = MapInput(node->input(0));
  Emit(NewLoadInstruction(MapOutput(node), array_ptr, array_ptr,
                          lir::Value::SmallInt32(offset)));
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

// Arithmetic nodes
#define DEFINE_ARITHMETIC_TRANSLATOR(IrName, LirName)      \
  void Translator::Visit##IrName(ir::IrName##Node* node) { \
    auto const output = MapOutput(node);                   \
    auto const left = MapInput(node->input(0));            \
    auto const right = MapInput(node->input(1));           \
    Emit(New##LirName##Instruction(output, left, right));  \
  }

DEFINE_ARITHMETIC_TRANSLATOR(FloatAdd, FloatAdd)
DEFINE_ARITHMETIC_TRANSLATOR(FloatDiv, FloatDiv)
DEFINE_ARITHMETIC_TRANSLATOR(FloatMod, FloatMod)
DEFINE_ARITHMETIC_TRANSLATOR(FloatMul, FloatMul)
DEFINE_ARITHMETIC_TRANSLATOR(FloatSub, FloatSub)
DEFINE_ARITHMETIC_TRANSLATOR(IntAdd, Add)
DEFINE_ARITHMETIC_TRANSLATOR(IntBitAnd, BitAnd)
DEFINE_ARITHMETIC_TRANSLATOR(IntBitOr, BitOr)
DEFINE_ARITHMETIC_TRANSLATOR(IntBitXor, BitXor)
DEFINE_ARITHMETIC_TRANSLATOR(IntDiv, IntDiv)
DEFINE_ARITHMETIC_TRANSLATOR(IntMod, IntMod)
DEFINE_ARITHMETIC_TRANSLATOR(IntMul, Mul)
DEFINE_ARITHMETIC_TRANSLATOR(IntShl, Shl)
DEFINE_ARITHMETIC_TRANSLATOR(IntSub, Sub)
DEFINE_ARITHMETIC_TRANSLATOR(UIntDiv, UIntDiv)
DEFINE_ARITHMETIC_TRANSLATOR(UIntMod, UIntMod)

// Simple nodes with three inputs

// data = load effect, anchor, pointer
void Translator::VisitLoad(ir::LoadNode* node) {
  auto const element_type = MapType(node->output_type());
  auto const output_type = PromoteType(element_type);
  auto const anchor = MapInput(node->input(1));
  auto const pointer = MapInput(node->input(2));
  auto const offset = lir::Value::SmallInt32(0);

  if (output_type.size == element_type.size)
    return Emit(NewLoadInstruction(MapOutput(node), anchor, pointer, offset));

  auto const element_value = NewRegister(element_type);
  Emit(NewLoadInstruction(element_value, anchor, pointer, offset));
  auto const output = MapOutput(node);
  if (node->output_type()->is_signed())
    return Emit(NewSignExtendInstruction(output, element_value));
  Emit(NewZeroExtendInstruction(output, element_value));
}

// control = ret control, effect, data
void Translator::VisitRet(ir::RetNode* node) {
  auto const value = node->input(2);
  if (value->is<ir::VoidNode>()) {
    editor()->SetReturn();
    return;
  }
  auto const input = MapInput(value);
  auto const return_type = PromoteType(input);
  auto const output = lir::Target::ReturnAt(return_type, 0);
  DCHECK_LE(lir::Value::SizeOf(return_type), lir::Value::SizeOf(output))
      << return_type << " " << output;

  if (output.size == input.size || !input.is_output()) {
    EmitSetValue(output, value);
    editor()->SetReturn();
    return;
  }

  if (value->output_type()->is_signed()) {
    Emit(NewSignExtendInstruction(output, input));
    editor()->SetReturn();
    return;
  }

  Emit(NewZeroExtendInstruction(output, input));
  editor()->SetReturn();
}

// Simple node 4 inputs

// control(type) %control = Call(%control, %effect, %callee, %arguments)
void Translator::VisitCall(ir::CallNode* node) {
  auto const callee = MapInput(node->input(2));
  auto const argument = node->input(3);

  auto const return_type =
      MapType(node->output_type()->as<ir::ControlType>()->data_type());
  std::vector<lir::Value> returns;
  if (!return_type.is_void_type())
    returns.push_back(lir::Target::ReturnAt(PromoteType(return_type), 0));

  if (argument->output_type()->is<ir::VoidType>())
    return Emit(NewCallInstruction(returns, callee));

  auto const tuple = argument->as<ir::TupleNode>();
  if (!tuple) {
    auto const arg_type = MapType(argument->output_type());
    EmitSetValue(lir::Target::ArgumentAt(arg_type, 0), argument);
    Emit(NewCallInstruction(returns, callee));
    return;
  }

  std::vector<lir::Value> inputs;
  inputs.reserve(tuple->CountInputs());
  std::vector<lir::Value> outputs;
  outputs.reserve(tuple->CountInputs());
  auto position = static_cast<size_t>(0);
  for (auto const argument : tuple->inputs()) {
    auto const arg = MapInput(argument);
    inputs.push_back(arg);
    outputs.push_back(lir::Target::ArgumentAt(arg, position));
    ++position;
  }
  Emit(NewPCopyInstruction(outputs, inputs));
  Emit(NewCallInstruction(returns, callee));
}

// Variadic inputs node
void Translator::VisitCase(ir::CaseNode* node) {
  // nothing to do
}

void Translator::VisitTuple(ir::TupleNode* node) {
  // TODO(eval1749): NYI translate Tuple
  DCHECK(node->SelectUserIfOne()) << *node;
  DCHECK_EQ(ir::Opcode::Call, node->SelectUserIfOne()->opcode())
      << *node << " by " << *node->SelectUserIfOne();
}

// Non simple inputs node
void Translator::VisitEffectPhi(ir::EffectPhiNode* node) {
  // nothing to do
}

void Translator::VisitEntry(ir::EntryNode* node) {
  auto const parameters_type = node->parameters_type();
  if (parameters_type->is<ir::VoidType>())
    return;
  std::vector<lir::Value> inputs;
  std::vector<lir::Value> outputs;
  for (auto const edge : node->use_edges()) {
    auto const param = edge->from()->as<ir::ParameterNode>();
    if (!param)
      continue;
    auto const output = MapOutput(param);
    outputs.push_back(output);
    inputs.push_back(lir::Target::ParameterAt(output, param->field()));
  }
  Emit(NewPCopyInstruction(outputs, inputs));
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
  auto const output = NewConditional();
  DCHECK(!register_map_.count(node)) << *node;
  register_map_.insert(std::make_pair(node, output));

  auto const left = MapInput(node->input(0));
  auto const right = MapInput(node->input(1));
  // TODO(eval1749) We should emit |IfInstruction| when |IntCmpNode| isn't used
  // for |IfNode|.
  Emit(NewCmpInstruction(output, MapCondition(node->condition()), left, right));
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
  // |ParameterNode| is process by |EntryNode|
}

void Translator::VisitPhi(ir::PhiNode* node) {
  editor()->NewPhi(MapOutput(node));
}

void Translator::VisitReference(ir::ReferenceNode* node) {
  // TODO(eval1749): NYI translate Reference
  NOTREACHED() << *node;
}

void Translator::VisitSizeOf(ir::SizeOfNode* node) {
  // nothing to do
}

// effect = store effect, anchor, pointer, new_value
void Translator::VisitStore(ir::StoreNode* node) {
  auto const anchor = MapInput(node->input(1));
  auto const pointer = MapInput(node->input(2));
  auto const offset = lir::Value::SmallInt32(0);
  auto const new_value = MapInput(node->input(3));
  auto const element_type = MapType(node->input(3)->output_type());
  if (new_value.size == element_type.size)
    return Emit(New<lir::StoreInstruction>(anchor, pointer, offset, new_value));
  auto const element_value = NewRegister(element_type);
  Emit(NewTruncateInstruction(element_value, new_value));
  Emit(New<lir::StoreInstruction>(anchor, pointer, offset, element_value));
}

void Translator::VisitVoid(ir::VoidNode* node) {
  // nothing to do
}

}  // namespace translator
}  // namespace elang
