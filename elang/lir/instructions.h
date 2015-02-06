// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_H_
#define ELANG_LIR_INSTRUCTIONS_H_

#include <array>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/iterator_on_iterator.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

// See "instructions_forward.h" for list of all instructions.
// See "instructions_${arch}.cc" for implementations depend on ISA.

class BasicBlock;
class Editor;
class Factory;

//////////////////////////////////////////////////////////////////////
//
// Opcode
//
enum class Opcode {
#define V(Name, mnemonic, ...) Name,
  FOR_EACH_LIR_INSTRUCTION(V)
#undef V
};

//////////////////////////////////////////////////////////////////////
//
// BasicBlockOperands
//
class BasicBlockOperands final {
 public:
  class Iterator final {
   public:
    explicit Iterator(BasicBlock** pointer);
    Iterator(const Iterator& other);
    ~Iterator();

    Iterator& operator=(const Iterator& other);

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

    BasicBlock* operator*() { return *pointer_; }
    BasicBlock* operator->() { return *pointer_; }
    Iterator& operator++();

   private:
    BasicBlock** pointer_;
  };

  BasicBlockOperands(const BasicBlockOperands& other);
  BasicBlockOperands(BasicBlock** start, BasicBlock** end);
  BasicBlockOperands();
  ~BasicBlockOperands();

  BasicBlockOperands& operator=(const BasicBlockOperands& other);

  Iterator begin() const { return Iterator(start_); }
  bool empty() const { return start_ == end_; }
  Iterator end() const { return Iterator(end_); }
  int size() const { return static_cast<int>(end_ - start_); }

 private:
  friend class Instruction;

  BasicBlock** start_;
  BasicBlock** end_;
};

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
class ELANG_LIR_EXPORT Instruction
    : public Castable,
      public DoubleLinked<Instruction, BasicBlock>::Node,
      public Visitable<InstructionVisitor>,
      public ZoneAllocated {
  DECLARE_CASTABLE_CLASS(Instruction, Castable);

 public:
  class ELANG_LIR_EXPORT Values final {
   public:
    class ELANG_LIR_EXPORT Iterator final {
     public:
      Iterator(const Iterator& other);
      explicit Iterator(Value* pointer);
      ~Iterator();

      Iterator& operator=(const Iterator& other);

      Value operator*() { return *pointer_; }
      Value operator->() { return operator*(); }

      Iterator& operator++();

      bool operator==(const Iterator& other) const;
      bool operator!=(const Iterator& other) const;

     private:
      Value* pointer_;
    };

    Values(const Values& other);
    Values(Value* start, Value* end);
    ~Values();

    Values& operator=(const Values& other);

    Iterator begin() { return Iterator(start_); }
    bool empty() const { return start_ == end_; }
    Iterator end() { return Iterator(end_); }
    int size() const { return static_cast<int>(end_ - start_); }

   private:
    Value* start_;
    Value* end_;
  };

  // A basic block which this instruction belongs to
  BasicBlock* basic_block() const { return basic_block_; }

  // An integer identifier for debugging.
  int id() const { return id_; }

  // ISA independent mnemonic for printing and debugging, see also
  // |MnemonicOf(Opcode) -> base::StringPiece|.
  base::StringPiece mnemonic() const;

  // Operation code of this instruction.
  virtual Opcode opcode() const = 0;

  // Operands accessor
  Value input(int index) const;
  Values inputs() const;
  Value output(int index) const;
  Values outputs() const;
  virtual BasicBlockOperands block_operands() const;
  BasicBlock* block_operand(int index) const;

  virtual int CountInputs() const = 0;
  virtual int CountOutputs() const = 0;

  // Returns true if this instruction is placed at end of block, e.g. 'br',
  // 'br', 'switch', and so on.
  virtual bool IsTerminator() const;

 protected:
  Instruction();

  virtual Value* InputValues() const = 0;
  virtual Value* OutputValues() const = 0;

  void InitInput(int index, Value new_value);
  void InitOutput(int index, Value new_value);

 private:
  // |Editor| changes |basic_block_|, |id_|, and |opcode_|.
  friend class Editor;
  friend class Factory;

  void SetBlockOperand(int index, BasicBlock* new_value);
  void SetInput(int index, Value new_value);
  void SetOutput(int index, Value new_value);

  BasicBlock* basic_block_;
  int id_;

  DISALLOW_COPY_AND_ASSIGN(Instruction);
};

#define DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Name)      \
  DECLARE_CASTABLE_CLASS(Name##Instruction, Instruction); \
  DISALLOW_COPY_AND_ASSIGN(Name##Instruction);            \
  Opcode opcode() const final { return Opcode::Name; }    \
  void Accept(InstructionVisitor* visitor) override;      \
  friend class Editor;                                    \
  friend class Factory;

// InstructionTemplate
template <int kOutputCount, int kInputCount>
class InstructionTemplate : public Instruction {
 public:
  int CountInputs() const final { return kInputCount; }
  int CountOutputs() const final { return kOutputCount; }

 protected:
  InstructionTemplate() = default;

 private:
  Value* InputValues() const final {
    auto const self = const_cast<InstructionTemplate*>(this);
    return self->operands_.data() + kOutputCount;
  }

  Value* OutputValues() const final {
    auto const self = const_cast<InstructionTemplate*>(this);
    return self->operands_.data();
  }

  std::array<Value, kInputCount + kOutputCount> operands_;

  DISALLOW_COPY_AND_ASSIGN(InstructionTemplate);
};

template <>
class InstructionTemplate<0, 0> : public Instruction {
 public:
  int CountInputs() const final { return 0; }
  int CountOutputs() const final { return 0; }

 protected:
  InstructionTemplate() = default;

 private:
  Value* InputValues() const final { return nullptr; }
  Value* OutputValues() const final { return nullptr; }

  DISALLOW_COPY_AND_ASSIGN(InstructionTemplate);
};

template <int kInputOperands, int kBasicBlockOperands>
class TerminatorInstruction : public InstructionTemplate<0, kInputOperands> {
 public:
  BasicBlockOperands block_operands() const final {
    auto const self = const_cast<TerminatorInstruction*>(this);
    return BasicBlockOperands(
        self->block_operands_.data(),
        self->block_operands_.data() + kBasicBlockOperands);
  }

  bool IsTerminator() const { return true; }

 protected:
  void InitBasicBlock(int index, BasicBlock* block) {
    block_operands_[index] = block;
  }

 private:
  std::array<BasicBlock*, kBasicBlockOperands> block_operands_;
};

// BranchInstruction
class ELANG_LIR_EXPORT BranchInstruction final
    : public TerminatorInstruction<1, 2> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Branch);

 public:
  BasicBlock* false_block() const { return block_operand(1); }
  BasicBlock* true_block() const { return block_operand(0); }

 private:
  BranchInstruction(Value condition,
                    BasicBlock* true_block,
                    BasicBlock* false_block);
};

// EntryInstruction
class ELANG_LIR_EXPORT EntryInstruction final
    : public InstructionTemplate<0, 0> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Entry);

 private:
  EntryInstruction();
};

// ExitInstruction
class ELANG_LIR_EXPORT ExitInstruction final
    : public TerminatorInstruction<0, 0> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Exit);

 private:
  ExitInstruction();
};

// JumpInstruction
class ELANG_LIR_EXPORT JumpInstruction final
    : public TerminatorInstruction<0, 1> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Jump);

 public:
  BasicBlock* target_block() const { return block_operand(0); }

 private:
  explicit JumpInstruction(BasicBlock* target_block);
};

// PCopyInstruction - represents parallel copy "pseudo" instruction.
// Number of input and output operands can not be changed after construction
// and they are matched.
class ELANG_LIR_EXPORT PCopyInstruction final : public Instruction {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(PCopy);

 private:
  PCopyInstruction(Zone* zone,
                   const std::vector<Value>& outputs,
                   const std::vector<Value>& inputs);

  // Instruction operand protocol
  int CountInputs() const final;
  int CountOutputs() const final;
  Value* InputValues() const final;
  Value* OutputValues() const final;

  ZoneVector<Value> inputs_;
  ZoneVector<Value> outputs_;
};

// PhiInput
class ELANG_LIR_EXPORT PhiInput final
    : public DoubleLinked<PhiInput, PhiInstruction>::Node,
      public ZoneAllocated {
 public:
  PhiInput(BasicBlock* block, Value value);
  ~PhiInput() = delete;

  BasicBlock* basic_block() const { return basic_block_; }
  Value value() const { return value_; }

 private:
  friend class Editor;

  Value value_;
  BasicBlock* basic_block_;

  DISALLOW_COPY_AND_ASSIGN(PhiInput);
};

// PhiInstruction
class ELANG_LIR_EXPORT PhiInstruction final : public Instruction {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Phi);

 public:
  typedef DoubleLinked<PhiInput, PhiInstruction> PhiInputs;

  Value input_of(BasicBlock* block) const;
  const PhiInputs& phi_inputs() const { return phi_inputs_; }

 private:
  explicit PhiInstruction(Value output_value);

  PhiInput* FindPhiInputFor(BasicBlock* block) const;

  // Instruction operand protocol
  int CountInputs() const final;
  int CountOutputs() const final;
  Value* InputValues() const final;
  Value* OutputValues() const final;

  Value output_;
  PhiInputs phi_inputs_;
};

//////////////////////////////////////////////////////////////////////
//
// Help classes for |BasicBlock|.
//
typedef DoubleLinked<Instruction, BasicBlock> InstructionList;

// PhiInstructionList
class ELANG_LIR_EXPORT PhiInstructionList final {
 public:
  class ELANG_LIR_EXPORT Iterator
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

// RetInstruction
class ELANG_LIR_EXPORT RetInstruction final
    : public TerminatorInstruction<0, 0> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Ret);

 private:
  RetInstruction();
};

#define V(Name, ...)                              \
  class ELANG_LIR_EXPORT Name##Instruction final  \
      : public InstructionTemplate<0, 1> {        \
    DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Name); \
                                                  \
   private:                                       \
    Name##Instruction(Value pointer);             \
  };
FOR_EACH_LIR_INSTRUCTION_0_1(V)
#undef V

#define V(Name, ...)                                   \
  class ELANG_LIR_EXPORT Name##Instruction final       \
      : public InstructionTemplate<0, 2> {             \
    DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Name);      \
                                                       \
   private:                                            \
    Name##Instruction(Value pointer, Value new_value); \
  };
FOR_EACH_LIR_INSTRUCTION_0_2(V)
#undef V

#define V(Name, ...)                              \
  class ELANG_LIR_EXPORT Name##Instruction final  \
      : public InstructionTemplate<1, 1> {        \
    DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Name); \
                                                  \
   private:                                       \
    Name##Instruction(Value output, Value input); \
  };
FOR_EACH_LIR_INSTRUCTION_1_1(V)
#undef V

#define V(Name, ...)                                          \
  class ELANG_LIR_EXPORT Name##Instruction final              \
      : public InstructionTemplate<1, 2> {                    \
    DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Name);             \
                                                              \
   private:                                                   \
    Name##Instruction(Value output, Value left, Value right); \
  };
FOR_EACH_LIR_INSTRUCTION_1_2(V)
#undef V

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_H_
