// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_H_
#define ELANG_HIR_INSTRUCTIONS_H_

#include <ostream>

#include "base/basictypes.h"
#include "elang/hir/hir_export.h"
// TODO(eval1749) We should not include "hir/factory.h". It is required for
// |InstructionTemplate|.
#include "elang/hir/factory.h"
#include "elang/hir/instructions_forward.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

// See "instructions_forward.h" for list of all instructions.

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
class ELANG_HIR_EXPORT Instruction
    : public Value,
      public DoubleLinked<Instruction, BasicBlock>::Node {
  DECLARE_HIR_VALUE_CLASS(Instruction, Value);

 public:
  // Opcode for formatting
  enum Opcode {
#define V(Name, ...) Name,
    FOR_EACH_HIR_INSTRUCTION(V)
#undef V
  };

  // A basic block which this instruction belons to
  BasicBlock* basic_block() const { return basic_block_; }

  // An integer identifier for debugging.
  int id() const { return id_; }
  void set_id(int id);

  // Opcode for formatting and debugging
  virtual Opcode opcode() const = 0;

  // Type of value produced by this instruction.
  Type* output_type() const { return type(); }

  // Returns true if this instruction can be safely removed and doesn't change
  // execution result of containing function. No users for an output of
  // this instruction returns true except for this instruction has no side-
  // effect, e.g. call, store, and so on.
  virtual bool CanBeRemoved() const;

  // Returns number of operands which this instruction has. Other than 'phi',
  // 'pack', and 'switch', number of values is constant.
  virtual int CountOperands() const = 0;

  // Returns true if this instruction is placed at end of block, e.g. 'br',
  // 'br', 'switch', and so on.
  virtual bool IsTerminator() const;

  // Value accessor
  virtual Value* OperandAt(int index) const = 0;
  virtual void SetOperandAt(int index, Value* new_value) = 0;

 protected:
  explicit Instruction(Type* output_type);

 private:
  friend class BasicBlockEditor;

  // Value
  void Accept(ValueVisitor* visitor) override;

  BasicBlock* basic_block_;
  int id_;
  DoubleLinked<UseDefNode, Instruction> users_;

  DISALLOW_COPY_AND_ASSIGN(Instruction);
};

std::ostream& operator<<(std::ostream& ostream, const Instruction& instruction);

template <class Derived, typename... Params>
class InstructionTemplate : public Instruction {
 public:
  typedef InstructionTemplate BaseClass;

  static Derived* New(Factory* factory, Type* output_type, Params... params) {
    return new (factory->zone()) Derived(output_type, params...);
  }

  // Instruction
  int CountOperands() const override { return sizeof...(Params); }
  Value* OperandAt(int index) const override { return operans_[index].value(); }
  void SetOperandAt(int index, Value* new_value) override {
    operans_[index].SetValue(new_value);
  }

 protected:
  InstructionTemplate(Type* output_type, Params... params)
      : Instruction(output_type) {
    InitOperands(0, params...);
  }

 private:
  void InitOperands(int index) { __assume(index); }
  template <typename... Params>
  void InitOperands(int index, Value* value, Params... params) {
    operans_[index].Init(this, value);
    InitOperands(index + 1, params...);
  }
  EmbeddedContainer<UseDefNode, sizeof...(Params)> operans_;
  DISALLOW_COPY_AND_ASSIGN(InstructionTemplate);
};

#define DECLARE_HIR_INSTRUCTION_CLASS(Name)                \
  DECLARE_HIR_VALUE_CLASS(Name##Instruction, Instruction); \
  Opcode opcode() const final;

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
class ELANG_HIR_EXPORT CallInstruction
    : public InstructionTemplate<CallInstruction, Value*, Value*> {
  DECLARE_HIR_INSTRUCTION_CLASS(Call);

 public:
  // Instruction
  bool CanBeRemoved() const override;

 private:
  friend class BaseClass;

  CallInstruction(Type* output_type, Value* callee, Value* arguments);

  DISALLOW_COPY_AND_ASSIGN(CallInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// EntryInstruction
//
class ELANG_HIR_EXPORT EntryInstruction
    : public InstructionTemplate<EntryInstruction> {
  DECLARE_HIR_INSTRUCTION_CLASS(Entry);

 private:
  friend class BaseClass;

  explicit EntryInstruction(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(EntryInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ExitInstruction
//
class ELANG_HIR_EXPORT ExitInstruction
    : public InstructionTemplate<ExitInstruction> {
  DECLARE_HIR_INSTRUCTION_CLASS(Exit);

 private:
  friend class BaseClass;

  explicit ExitInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const override;

  DISALLOW_COPY_AND_ASSIGN(ExitInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
class ELANG_HIR_EXPORT ReturnInstruction
    : public InstructionTemplate<ReturnInstruction, Value*, BasicBlock*> {
  DECLARE_HIR_INSTRUCTION_CLASS(Return);

 private:
  friend class BaseClass;

  ReturnInstruction(Type* output_type, Value* value, BasicBlock* exit_block);

  // Instruction
  bool IsTerminator() const override;

  DISALLOW_COPY_AND_ASSIGN(ReturnInstruction);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_H_
