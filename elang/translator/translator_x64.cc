// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/translator/translator.h"

#include "elang/lir/editor.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace translator {

// control = ret control, effect, data
void Translator::VisitRet(ir::RetNode* node) {
  auto const value = node->input(2);
  if (value->is<ir::VoidNode>()) {
    editor()->SetReturn();
    return;
  }
  auto const primitive_type =
      value->output_type()->as<ir::PrimitiveValueType>();
  if (!primitive_type) {
    EmitSetValue(lir::Target::GetRegister(lir::isa::RAX), value);
    editor()->SetReturn();
    return;
  }

  if (primitive_type->is_float()) {
    if (primitive_type->bit_size() == 64)
      EmitSetValue(lir::Target::GetRegister(lir::isa::XMM0D), value);
    else
      EmitSetValue(lir::Target::GetRegister(lir::isa::XMM0S), value);
    editor()->SetReturn();
    return;
  }

  if (primitive_type->bit_size() == 64) {
    EmitSetValue(lir::Target::GetRegister(lir::isa::RAX), value);
    editor()->SetReturn();
    return;
  }

  auto const output = lir::Target::GetRegister(lir::isa::EAX);
  auto const input = MapInput(value);
  if (primitive_type->bit_size() == 32 || !input.is_register()) {
    EmitSetValue(output, value);
    editor()->SetReturn();
    return;
  }

  if (primitive_type->is_signed()) {
    Emit(NewSignExtendInstruction(output, input));
    editor()->SetReturn();
    return;
  }
  Emit(NewZeroExtendInstruction(output, input));
  editor()->SetReturn();
}

}  // namespace translator
}  // namespace elang
