// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_INSTRUCTIONS_H_
#define ELANG_LIR_INSTRUCTIONS_H_

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

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
  // A basic block which this instruction belongs to
  BasicBlock* basic_block() const { return basic_block_; }

  // An integer identifier for debugging.
  int id() const { return id_; }

  // ISA dependent mnemonic for printing and debugging
  virtual base::StringPiece mnemonic() const = 0;

  // Operands accessor
  const ZoneVector<Value>& inputs() const { return inputs_; }
  const ZoneVector<Value>& outputs() const { return outputs_; }

  // Returns true if this instruction is placed at end of block, e.g. 'br',
  // 'br', 'switch', and so on.
  virtual bool IsTerminator() const;

 protected:
  Instruction(Factory* factory, int output_count, int input_count);

  void InitInput(int index, Value new_value);
  void InitOutput(int index, Value new_value);

 private:
  // |Editor| changes|basic_block_|, |id_|, and |opcode_|.
  friend class Editor;
  friend class Factory;

  BasicBlock* basic_block_;
  int id_;
  ZoneVector<Value> inputs_;
  ZoneVector<Value> outputs_;

  DISALLOW_COPY_AND_ASSIGN(Instruction);
};

#define DECLARE_LIR_INSTRUCTION_CLASS(Name)          \
  DECLARE_CASTABLE_CLASS(Name, Instruction);         \
  DISALLOW_COPY_AND_ASSIGN(Name);                    \
  base::StringPiece mnemonic() const final;          \
  void Accept(InstructionVisitor* visitor) override; \
  friend class Factory;

// CallInstruction
class ELANG_LIR_EXPORT CallInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(CallInstruction);

 private:
  explicit CallInstruction(Factory* factory, Value callee);
};

// EntryInstruction
class ELANG_LIR_EXPORT EntryInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(EntryInstruction);

 private:
  explicit EntryInstruction(Factory* factory);
};

// ExitInstruction
class ELANG_LIR_EXPORT ExitInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(ExitInstruction);

 private:
  explicit ExitInstruction(Factory* factory);

  // Instruction
  bool IsTerminator() const final;
};

// JumpInstruction
class ELANG_LIR_EXPORT JumpInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(JumpInstruction);

 private:
  explicit JumpInstruction(Factory* factory, BasicBlock* target_block);

  // Instruction
  bool IsTerminator() const final;
};

// LoadInstruction
class ELANG_LIR_EXPORT LoadInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(LoadInstruction);

 private:
  explicit LoadInstruction(Factory* factory, Value output, Value input);
};

// RetInstruction
class ELANG_LIR_EXPORT RetInstruction final : public Instruction {
  DECLARE_LIR_INSTRUCTION_CLASS(RetInstruction);

 private:
  explicit RetInstruction(Factory* factory);

  // Instruction
  bool IsTerminator() const final;
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_INSTRUCTIONS_H_
