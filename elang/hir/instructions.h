// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_H_
#define ELANG_HIR_INSTRUCTIONS_H_

#include "base/basictypes.h"
#include "elang/hir/operands.h"

namespace elang {
namespace hir {

// See "instructions_forward.h" for list of all instructions.

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
class Instruction : public Operand,
                    public DoubleLinked<Instruction, Block>::Node {
 public:
  Type* output_type() const { return type(); }

  // Returns true if this instruction can be safely removed and doesn't change
  // execution result of containing function. No users for an output of
  // this instruction returns true except for this instruction has no side-
  // effect, e.g. call, store, and so on.
  virtual bool CanBeRemoved() const;

  // Returns number of operands which this instruction has. Other than 'phi',
  // 'pack', and 'switch', number of operands is constant.
  virtual int CountOperands() = 0;

  // Returns true if this transfers control to exit block, e.g. 'jump', 'ret',
  // and 'unreachable'.
  virtual bool IsExit() const;

  // Returns true if this instruction is placed at end of block, e.g. 'br',
  // 'br', 'switch', and so on.
  virtual bool IsTerminator() const;

  // Operand accessor
  virtual Operand* OperandAt(int index) const = 0;
  virtual void SetOperandAt(int index, Operand* new_operand) = 0;

 protected:
  explicit Instruction(Type* output_type);
  ~Instruction() override = default;

 private:
  DoubleLinked<UseDefNode, Instruction> users_;

  DISALLOW_COPY_AND_ASSIGN(Instruction);
};

template <class Derived, typename... Params>
class InstructionTemplate : public Instruction {
 public:
  typedef InstructionTemplate BaseClass;

  static Derived* New(Factory* factory, Type* output_type, Params... params) {
    return new (factory->zone()) Derived(factory, output_type, params...);
  }

  // Instruction
  int CountOperands() override { return sizeof...(Params); }
  Operand* OperandAt(int index) const override {
    return operands_[index].operand();
  }
  void SetOperandAt(int index, Operand* new_operand) override {
    operands_[index].SetOperand(new_operand);
  }

 protected:
  InstructionTemplate(Type* output_type, Params... params)
      : Instruction(output_type) {
    InitOperands(0, params...);
  }

 private:
  void InitOperands(int index) { __assume(index); }
  template <typename... Params>
  void InitOperands(int index, Operand* operand, Params... params) {
    operands_[index].Init(this, operand);
    InitOperands(index + 1, params...);
  }
  EmbeddedContainer<UseDefNode, sizeof...(Params)> operands_;
  DISALLOW_COPY_AND_ASSIGN(InstructionTemplate);
};

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
class CallInstruction
    : public InstructionTemplate<CallInstruction, Operand*, Operand*> {
  DECLARE_HIR_OPERAND_CLASS(CallInstruction, Instruction);

 public:
  // Instruction
  bool CanBeRemoved() const override;

 private:
  friend class BaseClass;

  CallInstruction(Factory* factory,
                  Type* output_type,
                  Operand* callee,
                  Operand* arguments);

  DISALLOW_COPY_AND_ASSIGN(CallInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
class ReturnInstruction
    : public InstructionTemplate<ReturnInstruction, Operand*> {
  DECLARE_HIR_OPERAND_CLASS(ReturnInstruction, Instruction);

 public:
  static ReturnInstruction* New(Factory* factory, Operand* value);

  // Instruction
  bool IsExit() const override;
  bool IsTerminator() const override;

 private:
  friend class BaseClass;

  ReturnInstruction(Factory* factory, Type* output_type, Operand* value);

  DISALLOW_COPY_AND_ASSIGN(ReturnInstruction);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_H_
