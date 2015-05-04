// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_CG_GENERATOR_H_
#define ELANG_CG_GENERATOR_H_

#include <memory>
#include <unordered_map>

#include "elang/base/zone_owner.h"
#include "elang/hir/instruction_visitor.h"
#include "elang/lir/factory_user.h"
#include "elang/lir/value.h"

namespace elang {
namespace hir {
class BasicBlock;
class Function;
class Literal;
class Type;
class Value;
}
namespace lir {
class BasicBlock;
class Editor;
class Factory;
enum class FloatCondition;
class Function;
class Instruction;
enum class IntCondition;
}
namespace cg {

//////////////////////////////////////////////////////////////////////
//
// Generator
//
class Generator final : public ZoneOwner,
                        public hir::InstructionVisitor,
                        public lir::FactoryUser {
 public:
  Generator(lir::Factory* factory, hir::Function* hir_function);
  ~Generator();

  lir::Function* Generate();

 private:
  lir::Editor* editor() const { return editor_.get(); }
  lir::Function* function() const;

  void Emit(lir::Instruction* instruction);
  void EmitCopy(lir::Value output, lir::Value input);
  void EmitSetValue(lir::Value output, hir::Value* input);
  lir::Value GenerateShl(lir::Value index, int shift_count);
  void HandleComparison(hir::Instruction* instr,
                        lir::IntCondition signed_condition,
                        lir::IntCondition unsigned_condition,
                        lir::FloatCondition float_condition);
  lir::BasicBlock* MapBlock(hir::BasicBlock* block);
  lir::Value MapInput(hir::Value* instr);
  lir::Value MapOutput(hir::Instruction* instr);
  lir::Value MapRegister(hir::Value* value);

  // Returns type template |lir::Value| based on |hir_type|.
  static lir::Value MapType(hir::Type* hir_type);

  // Returns newly created |lir::Function| based on |hir_function|.
  static lir::Function* NewFunction(lir::Factory* factor,
                                    hir::Function* hir_function);

  // hir::InstructionVisitor
  void DoDefaultVisit(hir::Instruction* instr) final;
  void VisitBranch(hir::BranchInstruction* instr) final;
  void VisitElement(hir::ElementInstruction* instr) final;
  void VisitEntry(hir::EntryInstruction* instr) final;
  void VisitExit(hir::ExitInstruction* instr) final;
  void VisitCall(hir::CallInstruction* instr) final;
  void VisitGet(hir::GetInstruction* instr) final;
  void VisitJump(hir::JumpInstruction* instr) final;
  void VisitLength(hir::LengthInstruction* instr) final;
  void VisitLoad(hir::LoadInstruction* instr) final;
  void VisitRet(hir::RetInstruction* instr) final;
  void VisitStaticCast(hir::StaticCastInstruction* instr) final;
  void VisitTuple(hir::TupleInstruction* instr) final;

#define V(Name, ...) void Visit##Name(hir::Name##Instruction* instr) final;
  FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
  FOR_EACH_BITWISE_BINARY_OPERATION(V)
  FOR_EACH_BITWISE_SHIFT_OPERATION(V)
  FOR_EACH_EQUALITY_OPERATION(V)
  FOR_EACH_RELATIONAL_OPERATION(V)
#undef V

  // Map |hir::BasicBlock| to |lir::BasicBlock|.
  std::unordered_map<hir::BasicBlock*, lir::BasicBlock*> block_map_;

  // Map |hir::Value| to |lir::Value|.
  std::unordered_map<hir::Value*, lir::Value> register_map_;
  const std::unique_ptr<lir::Editor> editor_;

  // A |hir::Function| begin translated into |lir::Function|.
  hir::Function* const hir_function_;

  DISALLOW_COPY_AND_ASSIGN(Generator);
};

}  // namespace cg
}  // namespace elang

#endif  // ELANG_CG_GENERATOR_H_
