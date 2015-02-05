// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_CG_GENERATOR_H_
#define ELANG_CG_GENERATOR_H_

#include <memory>
#include <unordered_map>

#include "elang/base/zone_owner.h"
#include "elang/hir/instruction_visitor.h"

namespace elang {
namespace hir {
class BasicBlock;
class Function;
class Literal;
class Value;
}
namespace lir {
class BasicBlock;
class Editor;
class Factory;
class Function;
class Instruction;
struct Value;
}
namespace cg {

//////////////////////////////////////////////////////////////////////
//
// Generator
//
class Generator final : public ZoneOwner, public hir::InstructionVisitor {
 public:
  Generator(lir::Factory* factory, hir::Function* hir_function);
  ~Generator();

  lir::Function* Generate();

 private:
  lir::Editor* editor() const { return editor_.get(); }
  lir::Factory* factory() const;
  lir::Function* function() const;

  lir::Value AllocateRegister(hir::Value* value, int min_bit_size);
  void EditBasicBlock(hir::BasicBlock* hir_block);
  void Emit(lir::Instruction* instruction);
  void EmitCopy(lir::Value output, lir::Value input);
  void EmitSetValue(lir::Value output, hir::Value* input);
  lir::Value MapInput(hir::Value* instr);
  lir::Value MapOutput(hir::Instruction* instr);
  lir::Value MapRegister(hir::Value* value, int min_bit_size);

  // hir::InstructionVisitor
  void VisitElement(hir::ElementInstruction* instr) final;
  void VisitEntry(hir::EntryInstruction* instr) final;
  void VisitCall(hir::CallInstruction* instr) final;
  void VisitLoad(hir::LoadInstruction* instr) final;
  void VisitRet(hir::RetInstruction* instr) final;

#define V(Name, ...) void Visit##Name(hir::Name##Instruction* instr) final;
  FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
  FOR_EACH_BITWISE_BINARY_OPERATION(V)
  FOR_EACH_BITWISE_SHIFT_OPERATION(V)
#undef V

  std::unordered_map<hir::BasicBlock*, lir::BasicBlock*> block_map_;
  std::unordered_map<hir::Value*, lir::Value> register_map_;
  const std::unique_ptr<lir::Editor> editor_;
  hir::Function* const hir_function_;

  DISALLOW_COPY_AND_ASSIGN(Generator);
};

}  // namespace cg
}  // namespace elang

#endif  // ELANG_CG_GENERATOR_H_
