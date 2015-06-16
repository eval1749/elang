// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>
#include <unordered_set>

#include "base/strings/stringprintf.h"
#include "elang/shell/disasm.h"
#include "elang/targets/instruction_x64.h"
#include "elang/targets/operand_x64.h"
#include "elang/vm/machine_code_function.h"

namespace elang {
namespace compiler {
namespace shell {

using Instruction = ::elang::targets::x64::Instruction;
using Operand = ::elang::targets::x64::Operand;

namespace {
std::unordered_set<int> ListLabels(const vm::MachineCodeFunction* function) {
  auto const code_start = function->code_start_for_testing();
  auto const code_end = code_start + function->code_size_for_testing();
  std::unordered_set<int> labels;
  for (auto runner = code_start; runner < code_end;) {
    auto const instr = Instruction::Decode(runner, code_end);
    if (!instr.IsValid())
      break;
    runner += instr.size();
    for (auto const operand : instr.operands()) {
      if (operand.type() != Operand::Type::Relative)
        continue;
      auto const offset = runner - code_start;
      labels.insert(static_cast<int>(offset + operand.detail()));
    }
  }
  return std::move(labels);
}

struct DisassembledInstruction {
  const Instruction* instruction;
  int offset;
  char label;
};

std::ostream& operator<<(std::ostream& ostream,
                         const DisassembledInstruction& wrapper) {
  const size_t kNumCodes = 6;
  auto& instr = *wrapper.instruction;
  size_t end_index = std::max(instr.size(), kNumCodes);
  for (size_t index = 0; index < end_index; ++index) {
    if (index % kNumCodes == 0) {
      if (index)
        ostream << std::endl
                << ' ';
      else
        ostream << wrapper.label;
      ostream << base::StringPrintf("%04X", wrapper.offset + index);
    }
    if (index < instr.size())
      ostream << base::StringPrintf(" %02X", instr.byte_at(index));
    else
      ostream << "   ";
    if (index != kNumCodes - 1)
      continue;
    ostream << " " << instr.mnemonic();
    auto separator = " ";
    for (auto const operand : instr.operands()) {
      ostream << separator;
      separator = ", ";
      if (operand.type() != Operand::Type::Relative) {
        ostream << operand;
        continue;
      }
      auto const target = wrapper.offset + instr.size() + operand.detail();
      ostream << base::StringPrintf("L%04X", static_cast<int>(target));
    }
  }
  return ostream << std::endl;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream,
                         const DisassembledMachineCodeFunction& wrapper) {
  auto const function = wrapper.function;
  auto const labels = ListLabels(function);

  auto const code_start = function->code_start_for_testing();
  auto const code_end = code_start + function->code_size_for_testing();
  for (auto runner = code_start; runner < code_end;) {
    auto const instr = Instruction::Decode(runner, code_end);
    if (!instr.IsValid())
      break;
    auto const offset = static_cast<int>(runner - code_start);
    auto const label = labels.count(offset) ? 'L' : ' ';
    ostream << DisassembledInstruction{&instr, offset, label};
    runner += instr.size();
  }
  return ostream << std::endl;
}

}  // namespace shell
}  // namespace compiler
}  // namespace elang
