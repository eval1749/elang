// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_H_
#define ELANG_LIR_INSTRUCTIONS_H_

#include <array>
#include <iterator>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/iterator_on_iterator.h"
#include "elang/base/visitable.h"
#include "elang/base/work_list.h"
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
// FloatCondition
// Make |CommuteCondition()| to simple, condition ^ 15, we assign constant
// to each condition.
//
#define FOR_EACH_FLOAT_CONDITION(V)        \
  V(OrderedEqual, "eq", 0)                 \
  V(OrderedGreaterThanOrEqual, "ge", 1)    \
  V(OrderedGreaterThan, "gt", 2)           \
  V(UnorderedGreaterThanOrEqual, "uge", 3) \
  V(UnorderedGreaterThan, "ugt", 4)        \
  V(UnorderedEqual, "ueq", 5)              \
  V(Invalid6, "invalid6", 6)               \
  V(Invalid7, "invalid7", 7)               \
  V(Invalid8, "invalid8", 8)               \
  V(Invalid9, "invalid9", 9)               \
  V(UnorderedNotEqual, "une", 10)          \
  V(UnorderedLessThanOrEqual, "ule", 11)   \
  V(UnorderedLessThan, "ult", 12)          \
  V(OrderedLessThanOrEqual, "le", 13)      \
  V(OrderedLessThan, "lt", 14)             \
  V(OrderedNotEqual, "ne", 15)

enum class FloatCondition {
#define V(Name, mnemonic, value) Name = value,
  FOR_EACH_FLOAT_CONDITION(V)
#undef V
};

inline FloatCondition CommuteCondition(FloatCondition condition) {
  return static_cast<FloatCondition>(static_cast<int>(condition) ^ 15);
}

//////////////////////////////////////////////////////////////////////
//
// IntCondition
// Make |CommuteCondition()| to simple, condition ^ 15, we assign constant
// to each condition.
//
#define FOR_EACH_INTEGER_CONDITION(V)     \
  V(Equal, "eq", 0)                       \
  V(SignedGreaterThanOrEqual, "ge", 1)    \
  V(SignedGreaterThan, "gt", 2)           \
  V(UnsignedGreaterThanOrEqual, "uge", 3) \
  V(UnsignedGreaterThan, "ugt", 4)        \
  V(Invalid5, "invalid5", 5)              \
  V(Invalid6, "invalid6", 6)              \
  V(Invalid7, "invalid7", 7)              \
  V(Invalid8, "invalid8", 8)              \
  V(Invalid9, "invalid9", 9)              \
  V(Invalid10, "invalid10", 10)           \
  V(UnsignedLessThanOrEqual, "ule", 11)   \
  V(UnsignedLessThan, "ult", 12)          \
  V(SignedLessThanOrEqual, "le", 13)      \
  V(SignedLessThan, "lt", 14)             \
  V(NotEqual, "ne", 15)

enum class IntCondition {
#define V(Name, mnemonic, value) Name = value,
  FOR_EACH_INTEGER_CONDITION(V)
#undef V
};

inline IntCondition CommuteCondition(IntCondition condition) {
  return static_cast<IntCondition>(static_cast<int>(condition) ^ 15);
}

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
class ELANG_LIR_EXPORT BasicBlockOperands final {
 public:
  class ELANG_LIR_EXPORT Iterator final
      : public std::iterator<std::forward_iterator_tag, BasicBlock*> {
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
  size_t size() const { return end_ - start_; }

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
      public DoubleLinked<Instruction, BasicBlock>::NodeBase,
      public Visitable<InstructionVisitor>,
      public WorkList<Instruction>::Item,
      public ZoneAllocated {
  DECLARE_CASTABLE_CLASS(Instruction, Castable);

 public:
  class ELANG_LIR_EXPORT Values final {
   public:
    class ELANG_LIR_EXPORT Iterator final
        : public std::iterator<std::forward_iterator_tag, Value> {
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
    size_t size() const { return end_ - start_; }

   private:
    Value* start_;
    Value* end_;
  };

  // A basic block which this instruction belongs to
  BasicBlock* basic_block() const { return basic_block_; }

  // An integer identifier for debugging.
  int id() const { return id_; }
  int index() const { return index_; }

  // ISA independent mnemonic for printing and debugging, see also
  // |MnemonicOf(Opcode) -> base::StringPiece|.
  virtual base::StringPiece mnemonic() const;

  // Operation code of this instruction.
  virtual Opcode opcode() const = 0;

  // Operands accessor
  Value input(int index) const;
  Values inputs() const;
  Value output(int index) const;
  Values outputs() const;
  virtual BasicBlockOperands block_operands() const;
  BasicBlock* block_operand(int index) const;

  virtual size_t CountInputs() const = 0;
  virtual size_t CountOutputs() const = 0;

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
  int index_;

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
  size_t CountInputs() const final { return kInputCount; }
  size_t CountOutputs() const final { return kOutputCount; }

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
  size_t CountInputs() const final { return 0; }
  size_t CountOutputs() const final { return 0; }

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
  void InitBlockOperand(int index, BasicBlock* block) {
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

// CmpInstruction
class ELANG_LIR_EXPORT CmpInstruction final : public InstructionTemplate<1, 2> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Cmp);

 public:
  IntCondition condition() const { return condition_; }

 private:
  CmpInstruction(Value output, IntCondition condition, Value left, Value right);

  base::StringPiece mnemonic() const final;

  void set_condition(IntCondition new_condition) { condition_ = new_condition; }

  IntCondition condition_;
};

// EntryInstruction
class ELANG_LIR_EXPORT EntryInstruction final : public Instruction {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Entry);

 private:
  EntryInstruction(Zone* zone, const std::vector<Value>& outputs);

  // Instruction operand protocol
  size_t CountInputs() const final;
  size_t CountOutputs() const final;
  Value* InputValues() const final;
  Value* OutputValues() const final;

  ZoneVector<Value> outputs_;
};

// ExitInstruction
class ELANG_LIR_EXPORT ExitInstruction final
    : public TerminatorInstruction<0, 0> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Exit);

 private:
  ExitInstruction();
};

// FCmpInstruction
class ELANG_LIR_EXPORT FCmpInstruction final
    : public InstructionTemplate<1, 2> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(FCmp);

 public:
  FloatCondition condition() const { return condition_; }

 private:
  FCmpInstruction(Value output,
                  FloatCondition condition,
                  Value left,
                  Value right);

  base::StringPiece mnemonic() const final;

  void set_condition(FloatCondition new_condition) {
    condition_ = new_condition;
  }

  FloatCondition condition_;
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
  size_t CountInputs() const final;
  size_t CountOutputs() const final;
  Value* InputValues() const final;
  Value* OutputValues() const final;

  ZoneVector<Value> inputs_;
  ZoneVector<Value> outputs_;
};

// PhiInput
class ELANG_LIR_EXPORT PhiInput final
    : public DoubleLinked<PhiInput, PhiInstruction>::NodeBase,
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

  // Returns |PhiInput| for |block|. |this| instruction must have an operand
  // for |block|.
  Value input_of(BasicBlock* block) const;

  // Returns list of |PhiInput|.
  const PhiInputs& phi_inputs() const { return phi_inputs_; }

 private:
  explicit PhiInstruction(Value output_value);

  // Returns |PhiInput| if |this| instruction operand for |block|, otherwise
  // |nullptr|.
  PhiInput* FindPhiInputFor(BasicBlock* block) const;

  // Instruction operand protocol
  size_t CountInputs() const final;
  size_t CountOutputs() const final;
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
  bool empty() const { return list_->empty(); }

 private:
  const InstructionList* list_;
};

// RetInstruction
class ELANG_LIR_EXPORT RetInstruction final
    : public TerminatorInstruction<0, 1> {
  DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Ret);

 private:
  explicit RetInstruction(BasicBlock* exit_block);
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

#define V(Name, ...)                                                           \
  class ELANG_LIR_EXPORT Name##Instruction final                               \
      : public InstructionTemplate<1, 3> {                                     \
    DECLARE_CONCRETE_LIR_INSTRUCTION_CLASS(Name);                              \
                                                                               \
   private:                                                                    \
    Name##Instruction(Value output, Value input0, Value input1, Value input2); \
  };
FOR_EACH_LIR_INSTRUCTION_1_3(V)
#undef V

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_H_
