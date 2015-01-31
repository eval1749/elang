// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/instruction_factory.h"

#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/instruction_visitor.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace hir {

#define V(Name, ...)                                                     \
  void InstructionVisitor::Visit##Name(Name##Instruction* instruction) { \
    DoDefaultVisit(instruction);                                         \
  }
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

#define V(Name, ...)                                            \
  void Name##Instruction::Accept(InstructionVisitor* visitor) { \
    visitor->Visit##Name(this);                                 \
  }
FOR_EACH_HIR_INSTRUCTION(V)
#undef V

void InstructionVisitor::DoDefaultVisit(Instruction* instruction) {
  DCHECK(instruction);
}

InstructionFactory::InstructionFactory(Factory* factory,
                                       const FactoryConfig& config)
    : TypeFactoryUser(new TypeFactory(config)),
      factory_(factory),
      type_factory_(types()) {
}

InstructionFactory::~InstructionFactory() {
}

#define V(Name, ...)                                                 \
  Instruction* InstructionFactory::New##Name##Instruction(           \
      Type* output_type, Value* left, Value* right) {                \
    DCHECK(output_type->is_numeric()) << *output_type;               \
    DCHECK_EQ(output_type, left->type()) << *left << " " << *right;  \
    DCHECK_EQ(output_type, right->type()) << *left << " " << *right; \
    auto const instr = new (zone()) Name##Instruction(output_type);  \
    instr->InitInputAt(0, left);                                     \
    instr->InitInputAt(1, right);                                    \
    return instr;                                                    \
  }
FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
#undef V

#define V(Name, ...)                                                 \
  Instruction* InstructionFactory::New##Name##Instruction(           \
      Type* output_type, Value* left, Value* right) {                \
    DCHECK(output_type->is_integer()) << *output_type;               \
    DCHECK_EQ(output_type, left->type()) << *left << " " << *right;  \
    DCHECK_EQ(output_type, right->type()) << *left << " " << *right; \
    auto const instr = new (zone()) Name##Instruction(output_type);  \
    instr->InitInputAt(0, left);                                     \
    instr->InitInputAt(1, right);                                    \
    return instr;                                                    \
  }
FOR_EACH_BITWISE_BINARY_OPERATION(V)
#undef V

#define V(Name, ...)                                                \
  Instruction* InstructionFactory::New##Name##Instruction(          \
      Type* output_type, Value* left, Value* right) {               \
    DCHECK(output_type->is_integer()) << *output_type;              \
    DCHECK(left->type()->is_integer()) << *left << " " << *right;   \
    DCHECK_EQ(int32_type(), right->type()) << *right;               \
    auto const instr = new (zone()) Name##Instruction(output_type); \
    instr->InitInputAt(0, left);                                    \
    instr->InitInputAt(1, right);                                   \
    return instr;                                                   \
  }
FOR_EACH_BITWISE_SHIFT_OPERATION(V)
#undef V

#define V(Name, ...)                                                      \
  Instruction* InstructionFactory::New##Name##Instruction(Value* left,    \
                                                          Value* right) { \
    DCHECK_EQ(left->type(), right->type()) << *left << " " << *right;     \
    auto const instr = new (zone()) Name##Instruction(bool_type());       \
    instr->InitInputAt(0, left);                                          \
    instr->InitInputAt(1, right);                                         \
    return instr;                                                         \
  }
FOR_EACH_EQUALITY_OPERATION(V)
#undef V

#define V(Name, ...)                                                      \
  Instruction* InstructionFactory::New##Name##Instruction(Value* left,    \
                                                          Value* right) { \
    DCHECK(left->type()->is_numeric()) << *left << " " << *right;         \
    DCHECK_EQ(left->type(), right->type()) << *left << " " << *right;     \
    auto const instr = new (zone()) Name##Instruction(bool_type());       \
    instr->InitInputAt(0, left);                                          \
    instr->InitInputAt(1, right);                                         \
    return instr;                                                         \
  }
FOR_EACH_RELATIONAL_OPERATION(V)
#undef V

#define V(Name, ...)                                                         \
  Instruction* InstructionFactory::New##Name##Instruction(Type* output_type, \
                                                          Value* input) {    \
    auto const instr = new (zone()) Name##Instruction(output_type);          \
    instr->InitInputAt(0, input);                                            \
    return instr;                                                            \
  }
FOR_EACH_TYPE_CAST_OPERATION(V)
#undef V

Instruction* InstructionFactory::NewBound(Value* array, Value* indexes) {
  auto const array_type =
      array->type()->as<PointerType>()->pointee()->as<ArrayType>();
  DCHECK(array_type);
  auto const instr = new (zone()) BoundInstruction(bool_type());
#ifndef NDEBUG
  if (array_type->rank() == 1) {
    DCHECK_EQ(int32_type(), indexes->type());
  } else if (auto const indexes_type = indexes->as<TupleType>()) {
    DCHECK_EQ(array_type->rank(), indexes_type->size());
    for (auto const type : indexes_type->members())
      DCHECK_EQ(int32_type(), type);
  }
#endif
  instr->InitInputAt(0, array);
  instr->InitInputAt(1, indexes);
  return instr;
}

Instruction* InstructionFactory::NewBranchInstruction(Value* condition,
                                                      BasicBlock* true_block,
                                                      BasicBlock* false_block) {
  DCHECK_EQ(bool_type(), condition->type());
  auto const instr = new (zone()) BranchInstruction(void_type());
  instr->InitInputAt(0, condition);
  instr->InitInputAt(1, true_block);
  instr->InitInputAt(2, false_block);
  return instr;
}

Instruction* InstructionFactory::NewBranchInstruction(
    BasicBlock* target_block) {
  auto const instr = new (zone()) JumpInstruction(void_type());
  instr->InitInputAt(0, target_block);
  return instr;
}

Instruction* InstructionFactory::NewCallInstruction(Value* callee,
                                                    Value* arguments) {
  auto const callee_type = callee->type()->as<FunctionType>();
  DCHECK(callee_type);
  auto const instr = new (zone()) CallInstruction(callee_type->return_type());
  instr->InitInputAt(0, callee);
  instr->InitInputAt(1, arguments);
  return instr;
}

Instruction* InstructionFactory::NewEntryInstruction(Type* output_type) {
  return new (zone()) EntryInstruction(output_type);
}

Instruction* InstructionFactory::NewElement(Value* array, Value* indexes) {
  auto const array_type =
      array->type()->as<PointerType>()->pointee()->as<ArrayType>();
  DCHECK(array_type);
  auto const instr = new (zone())
      ElementInstruction(types()->NewPointerType(array_type->element_type()));
#ifndef NDEBUG
  if (array_type->rank() == 1) {
    DCHECK_EQ(int32_type(), indexes->type());
  } else if (auto const indexes_type = indexes->as<TupleType>()) {
    DCHECK_EQ(array_type->rank(), indexes_type->size());
    for (auto const type : indexes_type->members())
      DCHECK_EQ(int32_type(), type);
  }
#endif
  instr->InitInputAt(0, array);
  instr->InitInputAt(1, indexes);
  return instr;
}

Instruction* InstructionFactory::NewExitInstruction() {
  return new (zone()) ExitInstruction(void_type());
}

Instruction* InstructionFactory::NewGetInstruction(Value* value, int index) {
  auto const tuple_type = value->type()->as<TupleType>();
  DCHECK(tuple_type) << *value;
  auto const output_type = tuple_type->get(index);
  auto const instr = new (zone()) GetInstruction(output_type, index);
  instr->InitInputAt(0, value);
  return instr;
}

Instruction* InstructionFactory::NewIfInstruction(Type* output_type,
                                                  Value* condition,
                                                  Value* true_value,
                                                  Value* false_value) {
  DCHECK_EQ(bool_type(), condition->type());
  DCHECK_EQ(output_type, true_value->type());
  DCHECK_EQ(output_type, false_value->type());
  auto const instr = new (zone()) IfInstruction(output_type);
  instr->InitInputAt(0, condition);
  instr->InitInputAt(1, true_value);
  instr->InitInputAt(2, false_value);
  return instr;
}

Instruction* InstructionFactory::NewLoadInstruction(Value* pointer) {
  auto const pointer_type = pointer->type()->as<PointerType>();
  DCHECK(pointer_type);
  auto const instr = new (zone()) LoadInstruction(pointer_type->pointee());
  instr->InitInputAt(0, pointer);
  return instr;
}

PhiInstruction* InstructionFactory::NewPhiInstruction(Type* output_type) {
  return new (zone()) PhiInstruction(output_type);
}

Instruction* InstructionFactory::NewRetInstruction(Value* value,
                                                   BasicBlock* exit_block) {
  auto const instr = new (zone()) RetInstruction(void_type());
  instr->InitInputAt(0, value);
  instr->InitInputAt(1, exit_block);
  return instr;
}

Instruction* InstructionFactory::NewThrow(Value* value,
                                          BasicBlock* exit_block) {
  DCHECK(!value->is<VoidValue>());
  auto const instr = new (zone()) ThrowInstruction(void_type());
  instr->InitInputAt(0, value);
  instr->InitInputAt(1, exit_block);
  return instr;
}

Instruction* InstructionFactory::NewStackAlloc(Type* type, int count) {
  DCHECK(type->can_allocate_on_stack());
  DCHECK_GE(count, 1);
  return new (zone())
      StackAllocInstruction(types()->NewPointerType(type), count);
}

Instruction* InstructionFactory::NewStoreInstruction(Value* pointer,
                                                     Value* value) {
  DCHECK(pointer->type()->is<PointerType>());
  DCHECK(!value->type()->is<VoidType>());
  auto const instr = new (zone()) StoreInstruction(void_type());
  instr->InitInputAt(0, pointer);
  instr->InitInputAt(1, value);
  return instr;
}

Instruction* InstructionFactory::NewTuple(Type* output_type,
                                          const std::vector<Value*>& inputs) {
  DCHECK(output_type->is<TupleType>());
  DCHECK(!inputs.empty());
  auto const instr = new (zone())
      TupleInstruction(zone(), output_type, static_cast<int>(inputs.size()));
  auto index = 0;
  for (auto const input : inputs) {
    instr->InitInputAt(index, input);
    ++index;
  }
  return instr;
}

Instruction* InstructionFactory::NewUnreachableInstruction(
    BasicBlock* exit_block) {
  auto const instr = new (zone()) UnreachableInstruction(void_type());
  instr->InitInputAt(0, exit_block);
  return instr;
}

}  // namespace hir
}  // namespace elang
