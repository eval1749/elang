// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTION_FACTORY_H_
#define ELANG_HIR_INSTRUCTION_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/instructions_forward.h"
#include "elang/hir/types_forward.h"
#include "elang/hir/values_forward.h"

namespace elang {
namespace hir {

class Factory;
struct FactoryConfig;

//////////////////////////////////////////////////////////////////////
//
// InstructionFactory
//
class ELANG_HIR_EXPORT InstructionFactory : public ZoneOwner {
 public:
  InstructionFactory(Factory* factory, const FactoryConfig& config);
  ~InstructionFactory() = default;

  // Convenience function to have 'bool' type.
  Type* bool_type() const;
  // |TypeFactory| entry point.
  TypeFactory* types() const { return type_factory_.get(); }
  // Convenience function to have 'void' type.
  Type* void_type() const;
  // Convenience function to have 'void' value.
  Value* void_value() const;

  // Instruction constructors
  Instruction* NewBranchInstruction(Value* condition,
                                    BasicBlock* true_block,
                                    BasicBlock* false_block);
  Instruction* NewBranchInstruction(BasicBlock* taget_block);
  Instruction* NewCallInstruction(Value* callee, Value* arguments);
  Instruction* NewDynamicCast(Type* output_type, Value* input);
  Instruction* NewEntryInstruction(Type* output_type);
  Instruction* NewExitInstruction();
  Instruction* NewLoadInstruction(Value* pointer);
  PhiInstruction* NewPhiInstruction(Type* output_type);
  Instruction* NewReturnInstruction(Value* value, BasicBlock* exit_block);
  Instruction* NewStaticCast(Type* output_type, Value* input);
  Instruction* NewStoreInstruction(Value* pointer, Value* value);

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
