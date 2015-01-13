// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/isa_x64.h"

namespace elang {
namespace lir {
namespace isa {

namespace {
//////////////////////////////////////////////////////////////////////
//
// MnemonicFactory
//
class MnemonicFactory final : public InstructionVisitor {
 public:
  MnemonicFactory() = default;
  ~MnemonicFactory() final = default;

  base::StringPiece GetMnemonic(const Instruction* instruction);

#define V(Name, ...) void Visit##Name(Name##Instruction* instruction) final;
  FOR_EACH_LIR_INSTRUCTION(V)
#undef V

 private:
  base::StringPiece mnemonic_;

  DISALLOW_COPY_AND_ASSIGN(MnemonicFactory);
};

base::StringPiece MnemonicFactory::GetMnemonic(const Instruction* instruction) {
  mnemonic_.clear();
  const_cast<Instruction*>(instruction)->Accept(this);
  DCHECK(!mnemonic_.empty());
  return mnemonic_;
}

void MnemonicFactory::VisitCall(CallInstruction* instruction) {
  __assume(instruction);
  mnemonic_ = "call";
}

void MnemonicFactory::VisitEntry(EntryInstruction* instruction) {
  __assume(instruction);
  mnemonic_ = "entry";
}

void MnemonicFactory::VisitExit(ExitInstruction* instruction) {
  __assume(instruction);
  mnemonic_ = "exit";
}

void MnemonicFactory::VisitReturn(ReturnInstruction* instruction) {
  __assume(instruction);
  mnemonic_ = "ret";
}

}  // namespace

base::StringPiece GetMnemonic(const Instruction* instruction) {
  MnemonicFactory factory;
  return factory.GetMnemonic(instruction);
}

}  // namespace isa
}  // namespace lir
}  // namespace elang
