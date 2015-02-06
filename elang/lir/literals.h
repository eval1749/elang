// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_LITERALS_H_
#define ELANG_LIR_LITERALS_H_

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/float_types.h"
#include "elang/base/graph/graph.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_allocated.h"
#include "elang/lir/literals_forward.h"

namespace elang {
namespace lir {

class Instruction;
class PhiInstruction;
class PhiInstructionList;

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
class ELANG_LIR_EXPORT Literal : public Castable,
                                 public Visitable<LiteralVisitor>,
                                 public ZoneAllocated {
  DECLARE_LIR_LITERAL_CLASS(Literal, Castable);

 protected:
  Literal();

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

#define DECLARE_LIR_CONCRETE_LITERAL_CLASS(self) \
  DECLARE_LIR_LITERAL_CLASS(self, Literal);      \
  DISALLOW_COPY_AND_ASSIGN(self);                \
                                                 \
 private:                                        \
  void Accept(LiteralVisitor* visitor);

//////////////////////////////////////////////////////////////////////
//
// BasicBlock helpers
//
typedef DoubleLinked<Instruction, BasicBlock> InstructionList;

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
class ELANG_LIR_EXPORT BasicBlock final
    : public Literal,
      public Graph<Function, BasicBlock>::Node {
  DECLARE_LIR_CONCRETE_LITERAL_CLASS(BasicBlock);

 public:
  // An owner function
  Function* function() const { return function_; }

  // An integer identifier for debugging.
  int id() const { return id_; }

  // Getters for instructions in this basic block.
  Instruction* first_instruction() const;
  const InstructionList& instructions() const { return instructions_; }
  Instruction* last_instruction() const;
  PhiInstructionList phi_instructions() const;

  // For literal mapping.
  Value value() const { return value_; }

 private:
  // |Editor| manipulates instruction list
  friend class Editor;

  BasicBlock(Zone* zone, Value value);

  // |function_| holds owner of this |BasicBlock|.
  Function* function_;
  // |id_| is assign positive integer by |FunctionEditor|. When this basic
  // block is removed from |Function|, |id_| is reset to zero.
  int id_;
  // List of instructions.
  InstructionList instructions_;
  InstructionList phi_instructions_;
  Value const value_;
};

//////////////////////////////////////////////////////////////////////
//
// Function
//
class ELANG_LIR_EXPORT Function : public Literal,
                                  public Graph<Function, BasicBlock> {
  DECLARE_LIR_CONCRETE_LITERAL_CLASS(Function);

 public:
  const Nodes& basic_blocks() const { return nodes(); }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  int id() const;
  Value value() const { return value_; }

 private:
  friend class Editor;

  explicit Function(Value value);

  Value const value_;
};

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
