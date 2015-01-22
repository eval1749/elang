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

  TypeFactory* types() const { return type_factory_.get(); }
  VoidType* void_type() const;
  VoidValue* void_value() const;

  // Convenience function to have 'void' type.
  VoidType* GetVoidType() const { return void_type(); }

  // Instruction constructors
  BranchInstruction* NewBranchInstruction(Value* condition,
                                          BasicBlock* true_block,
                                          BasicBlock* false_block);
  CallInstruction* NewCallInstruction(Type* output_type,
                                      Value* callee,
                                      Value* arguments);
  EntryInstruction* NewEntryInstruction(Type* output_type);
  ExitInstruction* NewExitInstruction();
  ReturnInstruction* NewReturnInstruction(Value* value, BasicBlock* exit_block);

 private:
  Factory* const factory_;
  const std::unique_ptr<TypeFactory> type_factory_;

  DISALLOW_COPY_AND_ASSIGN(InstructionFactory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTION_FACTORY_H_
