// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_H_
#define ELANG_HIR_INSTRUCTIONS_H_

#include <array>

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
//
// Protocols for derived classes:
//  * Trait protocol
//   - opcode()
//   - IsTerminator()
//   - MaybeUseless()
//  * Inputs
//   - CountInputs()
//   - InputAt(int index)
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

  // Get |index|'th input value.
  Value* input(int index) const;

  // Accessing operands in this instruction.
  Operands inputs() const;

  // Type of value produced by this instruction.
  Type* output_type() const { return type(); }

  // Visitor pattern
  virtual void Accept(InstructionVisitor* visitor) = 0;

  // Returns number of operands which this instruction has. Other than 'phi',
  // 'tuple', and 'switch', number of values is constant.
  virtual int CountInputs() const = 0;

  // Returns true if this instruction is placed at end of block, e.g. 'br',
  // 'br', 'switch', and so on.
  virtual bool IsTerminator() const;

  // Returns true if this instruction has no users except and doesn't cause
  // side effect. 'call' and 'strore' instructions return false.
  virtual bool MaybeUseless() const;

 protected:
  explicit Instruction(Type* output_type);

 private:
  friend class Editor;
  friend class InstructionFactory;

  // Value
  bool is_alive() const final;

  // Protocol for accessing operands in each instruction implementation.
  virtual UseDefNode* InputAt(int index) const = 0;

  void InitInputAt(int index, Value* initial_value);
  void ResetInputAt(int index);
  void SetInputAt(int index, Value* new_value);

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
class SimpleInstruction : public Instruction {
 public:
  // Instruction
  int CountInputs() const final { return sizeof...(OperandTypes); }

 protected:
  explicit SimpleInstruction(Type* output_type) : Instruction(output_type) {}

 private:
  UseDefNode* InputAt(int index) const final {
    return const_cast<UseDefNode*>(&inputs_[index]);
  }

  std::array<UseDefNode, sizeof...(OperandTypes)> inputs_;

  DISALLOW_COPY_AND_ASSIGN(SimpleInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// Binary Operations
//
#define V(Name, ...)                                                  \
  class ELANG_HIR_EXPORT Name##Instruction final                      \
      : public SimpleInstruction<Name##Instruction, Value*, Value*> { \
    DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Name);                     \
                                                                      \
   private:                                                           \
    explicit Name##Instruction(Type* output_type);                    \
                                                                      \
    DISALLOW_COPY_AND_ASSIGN(Name##Instruction);                      \
  };

FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
FOR_EACH_BITWISE_BINARY_OPERATION(V)
FOR_EACH_BITWISE_SHIFT_OPERATION(V)
FOR_EACH_EQUALITY_OPERATION(V)
FOR_EACH_RELATIONAL_OPERATION(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Unary Operations
//
#define V(Name, ...)                                          \
  class ELANG_HIR_EXPORT Name##Instruction final              \
      : public SimpleInstruction<Name##Instruction, Value*> { \
    DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Name);             \
                                                              \
   private:                                                   \
    explicit Name##Instruction(Type* output_type);            \
                                                              \
    DISALLOW_COPY_AND_ASSIGN(Name##Instruction);              \
  };

FOR_EACH_TYPE_CAST_OPERATION(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// bool %out = bound %array %indexes
//
class ELANG_HIR_EXPORT BoundInstruction final
    : public SimpleInstruction<BoundInstruction, Value*, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Bound);

 private:
  explicit BoundInstruction(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(BoundInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// br %bool %true_block %false_block
// br %void %target_block
//
// |BranchInstruction| represents conditional branch.
//
class ELANG_HIR_EXPORT BranchInstruction final
    : public SimpleInstruction<BranchInstruction,
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
    : public SimpleInstruction<CallInstruction, Value*, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Call);

 private:
  explicit CallInstruction(Type* output_type);

  // Instruction
  bool MaybeUseless() const final;

  DISALLOW_COPY_AND_ASSIGN(CallInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %pointer = element %array %indexes
//
class ELANG_HIR_EXPORT ElementInstruction final
    : public SimpleInstruction<ElementInstruction, Value*, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Element);

 private:
  explicit ElementInstruction(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(ElementInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %parameters = entry
//
class ELANG_HIR_EXPORT EntryInstruction final
    : public SimpleInstruction<EntryInstruction> {
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
    : public SimpleInstruction<ExitInstruction> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Exit);

 private:
  explicit ExitInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const final;

  DISALLOW_COPY_AND_ASSIGN(ExitInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %result = get %tuple, index
//
class ELANG_HIR_EXPORT GetInstruction final
    : public SimpleInstruction<GetInstruction, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Get);

 public:
  int index() const { return index_; }

 private:
  explicit GetInstruction(Type* output_type, int index);

  int const index_;

  DISALLOW_COPY_AND_ASSIGN(GetInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %result if %bool %true, %false
//
class ELANG_HIR_EXPORT IfInstruction final
    : public SimpleInstruction<IfInstruction, Value*, Value*, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(If);

 private:
  explicit IfInstruction(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(IfInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// br block
//
class ELANG_HIR_EXPORT JumpInstruction final
    : public SimpleInstruction<JumpInstruction, BasicBlock*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Jump);

 public:
  BasicBlock* target_block() const;

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
    : public SimpleInstruction<LoadInstruction, Value*> {
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
// value. If it has just one pair, output of `phi` instruction should be
// replaced with its operand.
//
class ELANG_HIR_EXPORT PhiInstruction final : public Instruction {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Phi);

 public:
  typedef DoubleLinked<PhiInput, PhiInstruction> PhiInputs;

  Value* input_of(BasicBlock* block) const;
  const PhiInputs& phi_inputs() const { return phi_inputs_; }

 private:
  explicit PhiInstruction(Type* output_type);

  PhiInput* FindPhiInputFor(BasicBlock* block) const;

  int CountInputs() const final;
  UseDefNode* InputAt(int index) const final;

  PhiInputs phi_inputs_;

  DISALLOW_COPY_AND_ASSIGN(PhiInstruction);
};

// PhiInstructionList
class ELANG_HIR_EXPORT PhiInstructionList final {
 public:
  class ELANG_HIR_EXPORT Iterator
      : public IteratorOnIterator<Iterator, InstructionList::Iterator> {
   public:
    explicit Iterator(const InstructionList::Iterator& iterator);
    Iterator(const Iterator& other) = default;
    ~Iterator() = default;

    Iterator& operator=(const Iterator& other) = default;

    PhiInstruction* operator->() const { return operator*(); }
    PhiInstruction* operator*() const;
  };

  explicit PhiInstructionList(const InstructionList& list);
  PhiInstructionList(const PhiInstructionList& other) = default;
  ~PhiInstructionList() = default;

  PhiInstructionList& operator=(const PhiInstructionList& other) = default;

  Iterator begin() const;
  Iterator end() const;

 private:
  const InstructionList* list_;
};

//////////////////////////////////////////////////////////////////////
//
// void return %value, %exit_block
//
class ELANG_HIR_EXPORT RetInstruction final
    : public SimpleInstruction<RetInstruction, Value*, BasicBlock*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Ret);

 private:
  explicit RetInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const final;

  DISALLOW_COPY_AND_ASSIGN(RetInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// void store %pointer, %value
//
class ELANG_HIR_EXPORT StoreInstruction final
    : public SimpleInstruction<StoreInstruction, Value*, Value*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Store);

 private:
  explicit StoreInstruction(Type* output_type);

  // Instruction
  bool MaybeUseless() const final;

  DISALLOW_COPY_AND_ASSIGN(StoreInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// void throw %value, %exit_block
//
class ELANG_HIR_EXPORT ThrowInstruction final
    : public SimpleInstruction<ThrowInstruction, Value*, BasicBlock*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Throw);

 private:
  explicit ThrowInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const final;

  DISALLOW_COPY_AND_ASSIGN(ThrowInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %pointer = alloca count
//
class ELANG_HIR_EXPORT StackAllocInstruction final
    : public SimpleInstruction<StackAllocInstruction> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(StackAlloc);

 public:
  int count() const { return count_; }

 private:
  explicit StackAllocInstruction(Type* output_type, int count);

  int const count_;

  DISALLOW_COPY_AND_ASSIGN(StackAllocInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ty %tuple = tuple input+
//
class ELANG_HIR_EXPORT TupleInstruction final : public Instruction {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Tuple);

 private:
  explicit TupleInstruction(Zone* zone, Type* output_type, int count);

  // Instruction
  int CountInputs() const final;
  UseDefNode* InputAt(int index) const final;

  int count_;
  UseDefNode* inputs_;

  DISALLOW_COPY_AND_ASSIGN(TupleInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// unreachable %exit_block
//
class ELANG_HIR_EXPORT UnreachableInstruction final
    : public SimpleInstruction<UnreachableInstruction, BasicBlock*> {
  DECLARE_CONCRETE_HIR_INSTRUCTION_CLASS(Unreachable);

 private:
  explicit UnreachableInstruction(Type* output_type);

  // Instruction
  bool IsTerminator() const final;

  DISALLOW_COPY_AND_ASSIGN(UnreachableInstruction);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_H_
