// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

std::ostream& operator<<(std::ostream& ostream,
                         const DisassembleMachineCodeFunction& wrapper) {
  auto const mc_function = wrapper.function;
  auto const code_start = mc_function->code_start_for_testing();
  auto const code_end = code_start + mc_function->code_size_for_testing();
  std::unordered_set<int> labeled;
  for (auto runner = code_start; runner < code_end;) {
    auto const instr = Instruction::Decode(runner, code_end);
    if (!instr.IsValid())
      break;
    runner += instr.size();
    for (auto const operand : instr.operands()) {
      if (operand.type() != Operand::Type::Relative)
        continue;
      auto const offset = runner - code_start;
      labeled.insert(static_cast<int>(offset + operand.detail()));
    }
  }

  for (auto runner = code_start; runner < code_end;) {
    auto const instr = Instruction::Decode(runner, code_end);
    if (!instr.IsValid())
      break;
    auto const offset = static_cast<int>(runner - code_start);
    ostream << base::StringPrintf("%c%04X", labeled.count(offset) ? 'L' : ' ',
                                  offset);
    const size_t kNumCodes = 6;
    for (auto index = 0; index < instr.size(); ++index) {
      if (index && (index % kNumCodes == 0)) {
        ostream << std::endl
                << "     ";
      }
      ostream << base::StringPrintf(" %02X", instr.byte_at(index));
    }
    for (auto count = instr.size(); count % kNumCodes; ++count)
      ostream << "   ";
    ostream << " " << instr.mnemonic();
    auto separator = " ";
    for (auto const operand : instr.operands()) {
      ostream << separator;
      separator = ", ";
      if (operand.type() != Operand::Type::Relative) {
        ostream << operand;
        continue;
      }
      ostream << base::StringPrintf(
          "L%04X", static_cast<int>(offset + instr.size() + operand.detail()));
    }
    ostream << std::endl;
    runner += instr.size();
  }
  return ostream << std::endl;
}

}  // namespace shell
}  // namespace compiler
}  // namespace elang
