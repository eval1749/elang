// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_H_
#define ELANG_HIR_INSTRUCTIONS_H_

#include <array>
#include <vector>

#include "base/basictypes.h"
#include "base/logging.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/instructions_forward.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

// See "instructions_forward.h" for list of all instructions.

// Opcode for formatting
enum class Opcode {
#define V(Name, ...) Name,
  FOR_EACH_HIR_INSTRUCTION(V)
#undef V
};

#define DECLARE_HIR_INSTRUCTION_CLASS(Name)                \
  DECLARE_HIR_VALUE_CLASS(Name##Instruction, Instruction); \
  friend class Editor;                                     \
  friend class InstructionFactory;                         \
  friend class Validator;

#define DECLARE_ABSTRACT_HIR_INSTRUCTION_CLASS(Name) \
  DECLARE_HIR_INSTRUCTION_CLASS(Name);

#define DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Name) \
  DECLARE_HIR_INSTRUCTION_CLASS(Name);               \
  Opcode opcode() const final;                       \
  void Accept(InstructionVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// Operands
//
class ELANG_HIR_EXPORT Operands final {
 public:
  explicit Operands(const Instruction* instruction);
  Operands(const Operands& other);
  ~Operands();

  Operands& operator=(const Operands& other);

  OperandIterator begin();
  OperandIterator end();

 private:
  const Instruction* instruction_;
};

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
// Type of operands are guaranteed by |InstructionFactory| at construction,
// and |Validator| at modification.
//
class ELANG_HIR_EXPORT Instruction
    : public Value,
      public DoubleLinked<Instruction, BasicBlock>::Node {
  DECLARE_HIR_VALUE_CLASS(Instruction, Value);

 public:
  // A basic block which this instruction belongs to
  BasicBlock* basic_block() const { return basic_block_; }

  // A function which this instruction belongs to
  Function* function() const;

  // An integer identifier for debugging.
  int id() const { return id_; }

  // Opcode for formatting and debugging
  virtual Opcode opcode() const = 0;

  // Shortcut of |InputAt(index)|.
  Value* input(int index) const;

  // Accessing operands in this instruction.
  Operands inputs() const;

  // Type of value produced by this instruction.
  Type* output_type() const { return type(); }

  // Returns true if this instruction can be safely removed and doesn't change
  // execution result of containing function. No users for an output of
  // this instruction returns true except for this instruction has no side-
  // effect, e.g. call, store, and so on.
  virtual bool CanBeRemoved() const;

  // Returns number of operands which this instruction has. Other than 'phi',
  // 'tuple', and 'switch', number of values is constant.
  virtual int CountInputs() const = 0;

  // Returns true if this instruction is placed at end of block, e.g. 'br',
  // 'br', 'switch', and so on.
  virtual bool IsTerminator() const;

  virtual void Accept(InstructionVisitor* visitor) = 0;

 protected:
  explicit Instruction(Type* output_type);

 private:
  friend class Editor;

  // Protocol for accessing operands in each instruction implementation.
  virtual Value* InputAt(int index) const = 0;
  virtual void ResetInputAt(int index) = 0;
  virtual void SetInputAt(int index, Value* new_value) = 0;

  // Value
  void Accept(ValueVisitor* visitor) override;

  BasicBlock* basic_block_;
  int id_;
  DoubleLinked<UseDefNode, Instruction> users_;

  DISALLOW_COPY_AND_ASSIGN(Instruction);
};

//////////////////////////////////////////////////////////////////////
//
// Help template for fixed operands instructions.
//
template <class Derived, typename... OperandTypes>
class FixedOperandsInstruction : public Instruction {
 public:
  void InitInputAt(int index, Value* value) {
    inputs_[index].Init(this, value);
  }

  // Instruction
  int CountInputs() const final { return sizeof...(OperandTypes); }
  Value* InputAt(int index) const final { return inputs_[index].value(); }
  void ResetInputAt(int index) final { inputs_[index].Reset(); }
  void SetInputAt(int index, Value* new_value) final {
    DCHECK(new_value);
    inputs_[index].SetValue(new_value);
  }

 protected:
  explicit FixedOperandsInstruction(Type* output_type)
      : Instruction(output_type) {}

 private:
  std::array<UseDefNode, sizeof...(OperandTypes)> inputs_;

  DISALLOW_COPY_AND_ASSIGN(FixedOperandsInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// br %bool %true_block %false_block
// br %void %target_block
//
// |BranchInstruction| represents conditional branch.
//
class ELANG_HIR_EXPORT BranchInstruction final
    : public FixedOperandsInstruction<BranchInstruction,
                                      Value*,
                                      BasicBlock*,
                                      BasicBlock*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Branch);

 private:
  explicit BranchInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const final;

  DISALLOW_COPY_AND_ASSIGN(BranchInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %result = call funty %callee %arguments
//
class ELANG_HIR_EXPORT CallInstruction final
    : public FixedOperandsInstruction<CallInstruction, Value*, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Call);

 private:
  explicit CallInstruction(Type* output_type);

  // Instruction
  bool CanBeRemoved() const final;

  DISALLOW_COPY_AND_ASSIGN(CallInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %parameters = entry
//
class ELANG_HIR_EXPORT EntryInstruction final
    : public FixedOperandsInstruction<EntryInstruction> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Entry);

 private:
  explicit EntryInstruction(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(EntryInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// exit
//
class ELANG_HIR_EXPORT ExitInstruction final
    : public FixedOperandsInstruction<ExitInstruction> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Exit);

 private:
  explicit ExitInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const final;

  DISALLOW_COPY_AND_ASSIGN(ExitInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// jump block
//
class ELANG_HIR_EXPORT JumpInstruction final
    : public FixedOperandsInstruction<JumpInstruction, BasicBlock*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Jump);

 private:
  explicit JumpInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const final;

  DISALLOW_COPY_AND_ASSIGN(JumpInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %result = load %pointer
//
class ELANG_HIR_EXPORT LoadInstruction final
    : public FixedOperandsInstruction<LoadInstruction, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Load);

 private:
  explicit LoadInstruction(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(LoadInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// PhiInput
//
class ELANG_HIR_EXPORT PhiInput final
    : public DoubleLinked<PhiInput, PhiInstruction>::Node,
      public UseDefNode,
      public ZoneAllocated {
 public:
  PhiInput(PhiInstruction* phi, BasicBlock* block, Value* value);
  ~PhiInput() = delete;

  BasicBlock* basic_block() const { return basic_block_; }

 private:
  friend class Editor;

  BasicBlock* basic_block_;

  DISALLOW_COPY_AND_ASSIGN(PhiInput);
};

//////////////////////////////////////////////////////////////////////
//
// ty %result = phi %block1: %value1, ...
//
// `phi` instruction takes variable number of pairs of basic block and
// value. If it has just two operands, output of `phi` instruction should be
// replaced with its operand.
//
// Note: |PhiInstruction| isn't derived from |Instruction|.
//
class ELANG_HIR_EXPORT PhiInstruction final
    : public FixedOperandsInstruction<PhiInstruction> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Phi);

 public:
  typedef DoubleLinked<PhiInput, PhiInstruction> PhiInputs;

  Value* input_of(BasicBlock* block) const;
  const PhiInputs& phi_inputs() const { return phi_inputs_; }

 private:
  explicit PhiInstruction(Type* output_type);

  PhiInput* FindPhiInputFor(BasicBlock* block) const;

  PhiInputs phi_inputs_;

  DISALLOW_COPY_AND_ASSIGN(PhiInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// void return %value, %exit_block
//
class ELANG_HIR_EXPORT ReturnInstruction final
    : public FixedOperandsInstruction<ReturnInstruction, Value*, BasicBlock*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Return);

 private:
  explicit ReturnInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const final;

  DISALLOW_COPY_AND_ASSIGN(ReturnInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// void store %pointer, %value
//
class ELANG_HIR_EXPORT StoreInstruction final
    : public FixedOperandsInstruction<StoreInstruction, Value*, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Store);

 private:
  explicit StoreInstruction(Type* output_type);

  // Instruction
  bool CanBeRemoved() const final;

  DISALLOW_COPY_AND_ASSIGN(StoreInstruction);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_H_
