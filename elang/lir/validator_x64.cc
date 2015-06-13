// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <unordered_set>

#include "elang/lir/validator.h"

#include "base/logging.h"
#include "elang/lir/editor.h"
#include "elang/lir/error_code.h"
#include "elang/lir/error_data.h"
#include "elang/lir/instructions_x64.h"
#include "elang/lir/target.h"

namespace elang {
namespace lir {

void Validator::VisitUIntDivX64(UIntDivX64Instruction* instr) {
  auto const expected_output0 =
      Target::RegisterOf(instr->output(0).is_int32() ? isa::EAX : isa::RAX);
  auto const expected_output1 =
      Target::RegisterOf(instr->output(1).is_int32() ? isa::EDX : isa::RDX);
  if (instr->output(0) != expected_output0)
    Error(ErrorCode::ValidateInstructionOutput, instr, 0);
  if (instr->output(1) != expected_output1)
    Error(ErrorCode::ValidateInstructionOutput, instr, 1);
  if (instr->input(0) != expected_output0)
    Error(ErrorCode::ValidateInstructionInput, instr, 0);
  if (Value::TypeOf(instr->input(1)) != Value::TypeOf(instr->input(0)))
    Error(ErrorCode::ValidateInstructionInput, instr, 1);
}

void Validator::VisitUIntMulX64(UIntMulX64Instruction* instr) {
  auto const expected_output0 =
      Target::RegisterOf(instr->output(0).is_int32() ? isa::EAX : isa::RAX);
  auto const expected_output1 =
      Target::RegisterOf(instr->output(1).is_int32() ? isa::EDX : isa::RDX);
  if (instr->output(0) != expected_output0)
    Error(ErrorCode::ValidateInstructionOutput, instr, 0);
  if (instr->output(1) != expected_output1)
    Error(ErrorCode::ValidateInstructionOutput, instr, 1);
  if (instr->input(0) != expected_output0)
    Error(ErrorCode::ValidateInstructionInput, instr, 0);
  if (Value::TypeOf(instr->input(1)) != Value::TypeOf(instr->input(0)))
    Error(ErrorCode::ValidateInstructionInput, instr, 1);
}

}  // namespace lir
}  // namespace elang
