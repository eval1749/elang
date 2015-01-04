// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_LITERALS_H_
#define ELANG_LIR_LITERALS_H_

#include <ostream>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/embedded_container.h"
#include "elang/base/float_types.h"
#include "elang/base/zone_allocated.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/literals_forward.h"

namespace elang {
namespace lir {

class Instruction;

// Literal class hierarchy:
//    BasicBlock -- jump target
//    Float32Literal
//    Float64Literal
//    Function -- function literal
//    Int32Literal
//    Int64Literal
//    StringLiteral

#define DECLARE_LIR_LITERAL_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);         \
  friend class Factory;                        \
                                               \
 protected:                                    \
  ~self() override = default;

// Represent an value in instruction.
class ELANG_LIR_EXPORT Literal : public Castable, public ZoneAllocated {
  DECLARE_LIR_LITERAL_CLASS(Literal, Castable);

 public:
  virtual void Accept(LiteralVisitor* visitor);

 protected:
  Literal();

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

// Print for formatting and debugging.
ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Literal& literal);

#define DECLARE_LIR_CONCRETE_LITERAL_CLASS(self) \
  DECLARE_LIR_LITERAL_CLASS(self, Literal);      \
  DISALLOW_COPY_AND_ASSIGN(self);                \
                                                 \
 private:                                        \
  void Accept(LiteralVisitor* visitor);

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
// You can get predecessors of a basic block from user list in use-def list,
// and successors of a basic block from operands of the last instruction.
// Note: 'ret' and 'unreachable' instructions have 'exit' block as an operand.
//
// This class provides getters only. You need to use |Editor| for
// changing instructions, adding |BasicBlock| to |Function|, and so on.
//
// TODO(eval1749) Should we add |BasicBlock| to |Function| automatically when
// we insert 'jump' instruction?
//
class ELANG_LIR_EXPORT BasicBlock
    : public Literal,
      public DoubleLinked<BasicBlock, Function>::Node {
  DECLARE_LIR_CONCRETE_LITERAL_CLASS(BasicBlock);

 public:
  typedef DoubleLinked<Instruction, BasicBlock> InstructionList;

  // An owner function
  Function* function() const { return function_; }

  // An integer identifier for debugging.
  int id() const { return id_; }

  // Getters for instructions in this basic block.
  const InstructionList& instructions() const { return instructions_; }
  Instruction* first_instruction() const;
  Instruction* last_instruction() const;

 private:
  // |Editor| manipulates instruction list
  friend class Editor;

  BasicBlock();

  // |function_| holds owner of this |BasicBlock|.
  Function* function_;
  // |id_| is assign positive integer by |FunctionEditor|. When this basic
  // block is removed from |Function|, |id_| is reset to zero.
  int id_;
  // List of instructions.
  InstructionList instructions_;
};

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const BasicBlock& basic_block);

//////////////////////////////////////////////////////////////////////
//
// Function
//
class ELANG_LIR_EXPORT Function : public Literal {
  DECLARE_LIR_CONCRETE_LITERAL_CLASS(Function);

 public:
  typedef DoubleLinked<BasicBlock, Function> BasicBlockList;

  const BasicBlockList& basic_blocks() const { return basic_blocks_; }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;

 private:
  friend class Editor;

  explicit Function(Factory* factory);

  Function* function_;
  BasicBlockList basic_blocks_;
};

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Function& function);

//////////////////////////////////////////////////////////////////////
//
// Simple Literals
//
#define V(Name, name, cpp_type)                           \
  class ELANG_LIR_EXPORT Name##Literal : public Literal { \
    DECLARE_LIR_CONCRETE_LITERAL_CLASS(Name##Literal)     \
   public:                                                \
    explicit Name##Literal(cpp_type data);                \
                                                          \
    cpp_type data() const { return data_; }               \
                                                          \
   private:                                               \
    const cpp_type data_;                                 \
  };
FOR_EACH_LIR_SIMPLE_LITERAL(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// StirngLiteral
//
class ELANG_LIR_EXPORT StringLiteral final : public Literal {
  DECLARE_LIR_CONCRETE_LITERAL_CLASS(StringLiteral);

 public:
  base::StringPiece16 data() const { return data_; }

 private:
  explicit StringLiteral(base::StringPiece16 data);

  base::StringPiece16 data_;
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_LITERALS_H_
