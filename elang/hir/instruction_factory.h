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

  // |TypeFactory| entry point.
  TypeFactory* types() const { return type_factory_.get(); }
  // Convenience function to have 'void' type.
  VoidType* void_type() const;
  // Convenience function to have 'void' value.
  VoidValue* void_value() const;

  // Instruction constructors
  Instruction* NewBranchInstruction(Value* condition,
                                    BasicBlock* true_block,
                                    BasicBlock* false_block);
  Instruction* NewCallInstruction(Type* output_type,
                                  Value* callee,
                                  Value* arguments);
  Instruction* NewEntryInstruction(Type* output_type);
  Instruction* NewExitInstruction();
  Instruction* NewJumpInstruction(BasicBlock* target_block);
  Instruction* NewLoadInstruction(Type* output_type, Value* pointer);
  Instruction* NewReturnInstruction(Value* value, BasicBlock* exit_block);
  Instruction* NewStoreInstruction(Value* pointer, Value* value);

 private:
  Factory* const factory_;
  const std::unique_ptr<TypeFactory> type_factory_;

  DISALLOW_COPY_AND_ASSIGN(InstructionFactory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTION_FACTORY_H_
