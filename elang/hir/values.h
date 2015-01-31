// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_VALUES_H_
#define ELANG_HIR_VALUES_H_

#include <string>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/double_linked.h"
#include "elang/base/float_types.h"
#include "elang/base/visitable.h"
#include "elang/hir/thing.h"
#include "elang/hir/types_forward.h"
#include "elang/hir/values_forward.h"

namespace elang {
namespace hir {

// See "operands_forward.h" for list of all concrete operands.

// Value class hierarchy:
//  Value
//    BasicBlock -- jump target
//    Function -- function literal
//    Instruction -- output of |Instruction| represents SSA register.
//    Literal
//      BoolLiteral
//      CharLiteral
//      Float32Literal Float64Literal
//      Int16Literal Int32Literal Int64Literal Int16Literal
//      NullLiteral -- typed null value for pointer and reference types.
//      StringLiteral
//      UInt16Literal UInt32Literal UInt64Literal UInt16Literal
//      VoidValue -- singleton

// Use-def list node.
class ELANG_HIR_EXPORT UseDefNode
    : public DoubleLinked<UseDefNode, Value>::Node {
 public:
  UseDefNode();
  ~UseDefNode() = default;

  Instruction* instruction() const { return instruction_; }
  UseDefNode* next_user() const { return next(); }
  Value* value() const { return value_; }
  UseDefNode* previous_user() const { return previous(); }

 protected:
  void Init(Instruction* instruction, Value* initial_value);
  void Reset();
  void SetValue(Value* new_value);

 private:
  friend class Editor;

  // |Instruction| calls |Init| during construction.
  friend class Instruction;

  Value* value_;

  // Owner of this node which uses |value_|.
  Instruction* instruction_;

  DISALLOW_COPY_AND_ASSIGN(UseDefNode);
};

typedef DoubleLinked<UseDefNode, Value> UseDefList;

#define DECLARE_HIR_VALUE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);       \
  friend class Factory;                      \
                                             \
 protected:                                  \
  ~self() override = default;

//////////////////////////////////////////////////////////////////////
//
// Value - Represent an value in instruction.
//
class ELANG_HIR_EXPORT Value : public Thing, public Visitable<ValueVisitor> {
  DECLARE_HIR_VALUE_CLASS(Value, Thing);

 public:
  Type* type() const { return type_; }
  const UseDefList& users() const { return use_def_list_; }

  void Use(UseDefNode* holder);
  void Unuse(UseDefNode* holder);

 protected:
  explicit Value(Type* type);

 private:
  Type* const type_;
  UseDefList use_def_list_;

  DISALLOW_COPY_AND_ASSIGN(Value);
};

#define DECLARE_HIR_CONCRETE_VALUE_CLASS(self, super) \
  DECLARE_HIR_VALUE_CLASS(self, super);               \
                                                      \
 private:                                             \
  void Accept(ValueVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// |Literal| is a base class of literal value.
//
class ELANG_HIR_EXPORT Literal : public Value {
  DECLARE_HIR_VALUE_CLASS(Literal, Value);

 protected:
  explicit Literal(Type* type);

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

//////////////////////////////////////////////////////////////////////
//
// Typed null value. |TypeFactory| singleton.
//
class ELANG_HIR_EXPORT NullLiteral final : public Literal {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(NullLiteral, Literal);

 private:
  // For initializing per-type singleton null literal.
  friend class ArrayType;
  friend class PointerType;
  friend class ReferenceType;

  explicit NullLiteral(Type* type);

  DISALLOW_COPY_AND_ASSIGN(NullLiteral);
};

//////////////////////////////////////////////////////////////////////
//
// Represents reference to object.
//
class ELANG_HIR_EXPORT Reference final : public Literal {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(Reference, Literal);

 public:
  AtomicString* name() const { return name_; }

 private:
  Reference(Type* type, AtomicString* name);

  AtomicString* const name_;

  DISALLOW_COPY_AND_ASSIGN(Reference);
};

//////////////////////////////////////////////////////////////////////
//
// value for 'void' type. |TypeFactory| singleton.
//
class ELANG_HIR_EXPORT VoidValue final : public Literal {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(VoidValue, Literal);

 private:
  friend class VoidType;

  explicit VoidValue(VoidType* type);

  DISALLOW_COPY_AND_ASSIGN(VoidValue);
};

//////////////////////////////////////////////////////////////////////
//
// Primitive values
//
#define V(Name, name, data_type)                                \
  class ELANG_HIR_EXPORT Name##Literal final : public Literal { \
    DECLARE_HIR_CONCRETE_VALUE_CLASS(Name##Literal, Literal)    \
   public:                                                      \
    Name##Literal(Type* type, data_type data);                  \
    data_type data() const { return data_; }                    \
                                                                \
   private:                                                     \
    const data_type data_;                                      \
    DISALLOW_COPY_AND_ASSIGN(Name##Literal);                    \
  };
FOR_EACH_HIR_LITERAL_VALUE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// OperandIterator
//
class ELANG_HIR_EXPORT OperandIterator final {
 public:
  OperandIterator(const Instruction* instruction, int current);
  OperandIterator(const OperandIterator& other);
  ~OperandIterator();

  OperandIterator& operator=(const OperandIterator& other);
  OperandIterator& operator++();
  Value* operator*() const;
  Value* operator->() const;
  bool operator==(const OperandIterator& other) const;
  bool operator!=(const OperandIterator& other) const;

 private:
  const Instruction* instruction_;
  int current_;
};

//////////////////////////////////////////////////////////////////////
//
// Help classes for |BasicBlock|.
//
typedef DoubleLinked<Instruction, BasicBlock> InstructionList;

// IteratorOnIterator provides functions for implementing iterator on
// another iterator.
template <class Derived, class BaseIterator>
class IteratorOnIterator {
 public:
  IteratorOnIterator& operator=(const IteratorOnIterator& other) = default;

  Derived& operator++() {
    ++iterator_;
    return *static_cast<Derived*>(this);
  }

  bool operator==(const IteratorOnIterator& other) const {
    return iterator_ == other.iterator_;
  }

  bool operator!=(const IteratorOnIterator& other) const {
    return !operator==(other);
  }

 protected:
  explicit IteratorOnIterator(const BaseIterator& iterator)
      : iterator_(iterator) {}
  ~IteratorOnIterator() = default;

  const BaseIterator* iterator() const { return &iterator_; }
  BaseIterator* iterator() { return &iterator_; }

 private:
  BaseIterator iterator_;
};

// BasicBlockPredecessors
class ELANG_HIR_EXPORT BasicBlockPredecessors final {
 public:
  class ELANG_HIR_EXPORT Iterator final
      : public IteratorOnIterator<Iterator, UseDefList::Iterator> {
   public:
    explicit Iterator(const UseDefList::Iterator& iterator);
    Iterator(const Iterator& other) = default;
    ~Iterator() = default;

    Iterator& operator=(const Iterator& other) = default;

    BasicBlock* operator->() const { return operator*(); }
    BasicBlock* operator*() const;
  };

  explicit BasicBlockPredecessors(const BasicBlock* block);
  BasicBlockPredecessors(const BasicBlockPredecessors& other) = default;
  ~BasicBlockPredecessors() = default;

  BasicBlockPredecessors& operator=(const BasicBlockPredecessors& other) =
      default;

  Iterator begin() const;
  Iterator end() const;

 private:
  const BasicBlock* basic_block_;
};

// BasicBlockSuccessors
class ELANG_HIR_EXPORT BasicBlockSuccessors final {
 public:
  class ELANG_HIR_EXPORT Iterator final
      : public IteratorOnIterator<Iterator, OperandIterator> {
   public:
    explicit Iterator(const OperandIterator& iterator);
    Iterator(const Iterator& iterator) = default;
    ~Iterator() = default;

    BasicBlock* operator->() const { return operator*(); }
    BasicBlock* operator*() const;
  };

  explicit BasicBlockSuccessors(const BasicBlock* block);
  BasicBlockSuccessors(const BasicBlockSuccessors& other) = default;
  ~BasicBlockSuccessors() = default;

  BasicBlockSuccessors& operator=(const BasicBlockSuccessors& other) = default;

  Iterator begin() const;
  Iterator end() const;

 private:
  const BasicBlock* basic_block_;
};

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
// You can get predecessors of a basic block from user list in use-def list,
// and successors of a basic block from operands of the last instruction.
//
// For using use-def list for predecessors, we should do:
//  - 'ret' instructions have 'exit' block as an input operand.
//  - `switch` instructions have list of case target |BasicBlock| other than
//     input operands to handle multiple case labels point same |BasicBlock|.
//
// This class provides getters only. You need to use |Editor| for
// changing instructions, and |Editor| to add |BasicBlock| to
// |Function|.
//
class ELANG_HIR_EXPORT BasicBlock final
    : public Value,
      public DoubleLinked<BasicBlock, Function>::Node {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(BasicBlock, Value);

 public:
  Function* function() const { return function_; }

  // An integer identifier for debugging.
  int id() const { return id_; }

  // Getters for instructions in this basic block.
  const InstructionList& instructions() const { return instructions_; }
  Instruction* first_instruction() const;
  Instruction* last_instruction() const;
  PhiInstructionList phi_instructions() const;

  // Control flow graph
  BasicBlockPredecessors predecessors() const;
  BasicBlockSuccessors successors() const;

  bool HasMoreThanOnePredecessors() const;
  bool HasPredecessor() const;

 private:
  friend class Editor;

  explicit BasicBlock(Factory* factory);

  // |function_| holds owner of this |BasicBlock|.
  Function* function_;

  // |id_| is assign positive integer by |Editor|. When this basic
  // block is removed from |Function|, |id_| is reset to zero.
  int id_;

  // List of instructions in this block.
  InstructionList instructions_;

  // Phi instructions in this block.
  InstructionList phi_instructions_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};

//////////////////////////////////////////////////////////////////////
//
// Function
//
class ELANG_HIR_EXPORT Function final : public Value {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(Function, Value);

 public:
  typedef DoubleLinked<BasicBlock, Function> BasicBlockList;

  const BasicBlockList& basic_blocks() const { return basic_blocks_; }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  FunctionType* function_type() const;
  int id() const { return id_; }
  Type* parameters_type() const;
  Type* return_type() const;

 private:
  friend class Editor;

  Function(Factory* factory, FunctionType* type, int id);

  BasicBlockList basic_blocks_;
  int const id_;

  DISALLOW_COPY_AND_ASSIGN(Function);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_VALUES_H_
