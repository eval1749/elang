// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_H_
#define ELANG_LIR_INSTRUCTIONS_H_

#include <ostream>

#include "base/basictypes.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace isa {
enum class Opcode;
}

class BasicBlock;
class Factory;

// See "instructions_forward.h" for list of all instructions.

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
  // A basic block which this instruction belons to
  BasicBlock* basic_block() const { return basic_block_; }

  // An integer identifier for debugging.
  int id() const { return id_; }

  // Opcode for formatting and debugging
  isa::Opcode opcode() const { return opcode_; }

  // Operands accessor
  const ZoneVector<Value>& inputs() const { return inputs_; }
  const ZoneVector<Value>& outputs() const { return outputs_; }

  // Returns true if this instruction is placed at end of block, e.g. 'br',
  // 'br', 'switch', and so on.
  virtual bool IsTerminator() const;

 protected:
  explicit Instruction(Factory* factory,
                       isa::Opcode opcode,
                       int output_count,
                       int input_count);

 private:
  // |Editor| changes|basic_block_|, |id_|, and |opcode_|.
  friend class Editor;

  BasicBlock* basic_block_;
  int id_;
  isa::Opcode opcode_;
  ZoneVector<Value> inputs_;
  ZoneVector<Value> outputs_;

  DISALLOW_COPY_AND_ASSIGN(Instruction);
};

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Instruction& instruction);

#define DECLARE_LIR_INSTRUCTION_CLASS(Name)          \
  DECLARE_CASTABLE_CLASS(Name, Instruction);         \
  DISALLOW_COPY_AND_ASSIGN(Name);                    \
  void Accept(InstructionVisitor* visitor) override; \
  friend class Factory;

class ELANG_LIR_EXPORT CallInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(CallInstruction);

 private:
  explicit CallInstruction(Factory* factory);
};

class ELANG_LIR_EXPORT EntryInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(EntryInstruction);

 private:
  explicit EntryInstruction(Factory* factory);
};

class ELANG_LIR_EXPORT ExitInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(ExitInstruction);

 private:
  explicit ExitInstruction(Factory* factory);

  // Instruction
  bool IsTerminator() const final;
};

class ELANG_LIR_EXPORT ReturnInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(ReturnInstruction);

 private:
  explicit ReturnInstruction(Factory* factory);

  // Instruction
  bool IsTerminator() const final;
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_H_
