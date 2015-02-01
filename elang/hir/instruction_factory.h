// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTION_FACTORY_H_
#define ELANG_HIR_INSTRUCTION_FACTORY_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/instructions_forward.h"
#include "elang/hir/types_forward.h"
#include "elang/hir/type_factory_user.h"
#include "elang/hir/values_forward.h"

namespace elang {
namespace hir {

class Factory;
struct FactoryConfig;

//////////////////////////////////////////////////////////////////////
//
// InstructionFactory
//
class ELANG_HIR_EXPORT InstructionFactory : public TypeFactoryUser,
                                            public ZoneOwner {
 public:
  InstructionFactory(Factory* factory, const FactoryConfig& config);
  ~InstructionFactory();

  // Instruction constructors
  Instruction* NewBranchInstruction(Value* condition,
                                    BasicBlock* true_block,
                                    BasicBlock* false_block);
  Instruction* NewBranchInstruction(BasicBlock* target_block);
  Instruction* NewCallInstruction(Value* callee, Value* arguments);
  Instruction* NewEntryInstruction(Type* output_type);
  Instruction* NewBoundInstruction(Value* array, Value* indexes);
  Instruction* NewElementInstruction(Value* array, Value* indexes);
  Instruction* NewExitInstruction();
  Instruction* NewGetInstruction(Value* value, int index);
  Instruction* NewIfInstruction(Type* output_type,
                                Value* condition,
                                Value* true_value,
                                Value* false_value);
  Instruction* NewLoadInstruction(Value* pointer);
  PhiInstruction* NewPhiInstruction(Type* output_type);
  Instruction* NewRetInstruction(Value* value, BasicBlock* exit_block);
  Instruction* NewStackAllocInstruction(Type* type, int number_of_element);
  Instruction* NewStoreInstruction(Value* pointer, Value* value);
  Instruction* NewThrowInstruction(Value* value, BasicBlock* exit_block);
  Instruction* NewTupleInstruction(Type* output_type,
                                   const std::vector<Value*>& inputs);
  Instruction* NewUnreachableInstruction(BasicBlock* exit_block);

#define V(Name, ...)                                                  \
  Instruction* New##Name##Instruction(Type* output_type, Value* left, \
                                      Value* right);
  FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
  FOR_EACH_BITWISE_BINARY_OPERATION(V)
  FOR_EACH_BITWISE_SHIFT_OPERATION(V)
#undef V

#define V(Name, ...) \
  Instruction* New##Name##Instruction(Value* left, Value* right);
  FOR_EACH_EQUALITY_OPERATION(V)
  FOR_EACH_RELATIONAL_OPERATION(V)
#undef V

#define V(Name, ...) \
  Instruction* New##Name##Instruction(Type* output_type, Value* input);
  FOR_EACH_TYPE_CAST_OPERATION(V)
#undef V

 private:
  Factory* const factory_;
  const std::unique_ptr<TypeFactory> type_factory_;

  DISALLOW_COPY_AND_ASSIGN(InstructionFactory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTION_FACTORY_H_
