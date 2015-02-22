// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "elang/lir/emitters/code_buffer.h"
#include "elang/lir/emitters/code_buffer_user.h"
#include "elang/lir/emitters/code_emitter.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/isa_x64.h"
#include "elang/lir/literals.h"
#include "elang/lir/opcodes_x64.h"
#include "elang/lir/target_x64.h"
#include "elang/lir/target.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {

using isa::Mod;
using isa::Register;
using isa::Rm;
using isa::Rex;
using isa::Scale;
using isa::Tttn;

bool Is8Bit(int data) {
  return (data & 255) == data;
}

bool Is32Bit(int64_t data) {
  return (data & static_cast<uint32_t>(-1)) == data;
}

Value To32bitRegister(Value reg) {
  DCHECK(reg.is_physical());
  DCHECK_EQ(ValueSize::Size64, reg.size);
  return Value(reg.is_integer() ? Value::Type::Integer : Value::Type::Float,
               ValueSize::Size32, Value::Kind::PhysicalRegister, reg.data & 15);
}

isa::Register ToRegister(Value reg) {
  DCHECK(reg.is_physical());
  if (reg.is_float()) {
    if (reg.size == ValueSize::Size32)
      return static_cast<Register>(isa::XMM0S + (reg.data & 15));
    DCHECK_EQ(ValueSize::Size64, reg.size);
    return static_cast<Register>(isa::XMM0D + (reg.data & 15));
  }
  switch (reg.size) {
    case ValueSize::Size8:
      return static_cast<Register>(isa::AL + (reg.data & 15));
    case ValueSize::Size16:
      return static_cast<Register>(isa::AX + (reg.data & 15));
    case ValueSize::Size32:
      return static_cast<Register>(isa::EAX + (reg.data & 15));
    case ValueSize::Size64:
      return static_cast<Register>(isa::RAX + (reg.data & 15));
  }
  NOTREACHED() << "Unknown size: " << reg.size;
  return static_cast<Register>(0);
}

//////////////////////////////////////////////////////////////////////
//
// InstructionHandlerX64
//
class InstructionHandlerX64 final : public CodeBufferUser,
                                    public InstructionVisitor {
 public:
  InstructionHandlerX64(const Factory* factory, CodeBuffer* code_buffer);
  ~InstructionHandlerX64() final = default;

 private:
  void EmitModRm(Mod mod, Register reg, Register rm);
  void EmitModRm(Mod mod, Register reg, Rm rm);
  void EmitModRm(Register reg, Value input);
  void EmitModRm(Value output, isa::OpcodeExt opext);
  void EmitModRm(Value output, Value input);
  void EmitOpcode(isa::Opcode opcode);
  void EmitOpcode(isa::Opcode opcode, Register reg);
  void EmitOperand(Value value);
  void EmitRexPrefix(Value output, Value input);
  void EmitSib(Scale scale, Register index, Register base);

  int32_t Int32ValueOf(Value literal) const;
  bool Is32BitLiteral(Value literal) const;

  // InstructionVisitor
  void VisitCall(CallInstruction* instr) final;
  void VisitCopy(CopyInstruction* instr) final;
  void VisitLiteral(LiteralInstruction* instr) final;
  void VisitRet(RetInstruction* instr) final;

  const Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(InstructionHandlerX64);
};

InstructionHandlerX64::InstructionHandlerX64(const Factory* factory,
                                             CodeBuffer* code_buffer)
    : CodeBufferUser(code_buffer), factory_(factory) {
}

void InstructionHandlerX64::EmitModRm(Mod mod, Register reg, Register rm) {
  Emit8(static_cast<int>(mod) | ((reg & 7) << 3) | (rm & 7));
}

void InstructionHandlerX64::EmitModRm(Mod mod, Register reg, Rm rm) {
  EmitModRm(mod, reg, static_cast<Register>(rm));
}

void InstructionHandlerX64::EmitModRm(Register reg, Value memory) {
  if (memory.is_frame_slot()) {
    if (!memory.data) {
      // mov reg, [rbp]
      EmitModRm(Mod::Disp0, reg, isa::RBP);
      return;
    }
    // mov reg, [rbp+n]
    if (Is8Bit(memory.data)) {
      EmitModRm(Mod::Disp8, reg, isa::RBP);
      Emit8(memory.data);
      return;
    }
    // mov reg, [rbp+n]
    EmitModRm(Mod::Disp32, reg, isa::RBP);
    Emit32(memory.data);
    return;
  }
  if (memory.is_stack_slot()) {
    if (!memory.data) {
      EmitModRm(Mod::Disp0, reg, Rm::Sib);
      EmitSib(Scale::One, isa::RSP, isa::RSP);
      return;
    }
    if (Is8Bit(memory.data)) {
      // mov reg, [rsp+disp8]
      EmitModRm(Mod::Disp8, reg, Rm::Sib);
      EmitSib(Scale::One, isa::RSP, isa::RSP);
      Emit8(memory.data);
      return;
    }
    // mov reg, [rsp+n]
    EmitModRm(Mod::Disp32, reg, Rm::Sib);
    EmitSib(Scale::One, isa::RSP, isa::RSP);
    Emit32(memory.data);
    return;
  }
  NOTREACHED() << "EmitModRm " << reg << ", " << memory;
}

void InstructionHandlerX64::EmitModRm(Value output, isa::OpcodeExt opext) {
  EmitModRm(output, Target::GetRegister(static_cast<Register>(opext)));
}

void InstructionHandlerX64::EmitModRm(Value output, Value input) {
  if (output.is_physical()) {
    auto const reg = static_cast<Register>(output.data);
    if (input.is_physical()) {
      // mov reg1, reg2
      EmitModRm(Mod::Reg, reg, static_cast<Register>(input.data));
      return;
    }
    EmitModRm(reg, input);
    return;
  }
  if (input.is_physical()) {
    EmitModRm(static_cast<Register>(input.data), output);
    return;
  }
  NOTREACHED() << "EmitModRm " << output << ", " << input;
}

void InstructionHandlerX64::EmitOpcode(isa::Opcode opcode) {
  auto const value = static_cast<uint32_t>(opcode);
  DCHECK_LT(value, 1u << 24);
  if (value > 0xFFFF)
    Emit8(value >> 16);
  if (value > 0xFF)
    Emit8(value >> 8);
  Emit8(value);
}

void InstructionHandlerX64::EmitOpcode(isa::Opcode opcode, Register reg) {
  EmitOpcode(static_cast<isa::Opcode>(static_cast<int>(opcode) + (reg & 7)));
}

void InstructionHandlerX64::EmitOperand(Value value) {
  if (value.is_immediate()) {
    switch (value.size) {
      case ValueSize::Size8:
        Emit8(value.data);
        return;
      case ValueSize::Size16:
        Emit16(value.data);
        return;
      case ValueSize::Size32:
        Emit32(value.data);
        return;
      case ValueSize::Size64:
        Emit64(value.data);
        return;
    }
  }
  if (value.is_literal()) {
    auto const literal = factory_->GetLiteral(value);
    if (auto const i32 = literal->as<Int32Literal>())
      return Emit32(i32->data());
    if (auto const i64 = literal->as<Int64Literal>())
      return Emit64(i64->data());
  }
  AssociateValue(value);
  Emit32(0);
}

void InstructionHandlerX64::EmitRexPrefix(Value output, Value input) {
  if (output.size == ValueSize::Size16) {
    EmitOpcode(isa::Opcode::OPDSIZ);
    return;
  }
  int rex = isa::REX;
  if (output.size == ValueSize::Size64)
    rex |= isa::REX_W;
  if (output.is_physical() && output.data >= 8)
    rex |= isa::REX_R;
  if (input.is_physical() && input.data >= 8)
    rex |= isa::REX_B;
  if (rex == isa::REX)
    return;
  Emit8(rex);
}

void InstructionHandlerX64::EmitSib(Scale scale,
                                    Register index,
                                    Register base) {
  Emit8(static_cast<int>(scale) | ((index & 7) << 3) | (base & 7));
}

int32_t InstructionHandlerX64::Int32ValueOf(Value value) const {
  if (value.is_immediate())
    return value.data;
  DCHECK(value.is_literal());
  auto const literal = factory_->GetLiteral(value);
  if (auto const i32 = literal->as<Int32Literal>())
    return i32->data();
  if (auto const i64 = literal->as<Int64Literal>())
    return static_cast<int32_t>(i64->data());
  NOTREACHED() << value << " isn't 32-bit literal";
  return 0;
}

bool InstructionHandlerX64::Is32BitLiteral(Value value) const {
  if (value.is_immediate())
    return true;
  if (!value.is_literal())
    return false;
  auto const literal = factory_->GetLiteral(value);
  if (auto const i32 = literal->is<Int32Literal>())
    return true;
  if (auto const i64 = literal->as<Int64Literal>())
    return Is32Bit(i64->data());
  return false;
}

// InstructionVisitor
void InstructionHandlerX64::VisitCall(CallInstruction* instr) {
  EmitOpcode(isa::Opcode::CALL_Jv);
  EmitOperand(instr->input(0));
}

void InstructionHandlerX64::VisitCopy(CopyInstruction* instr) {
  auto const input = instr->input(0);
  auto const output = instr->output(0);
  DCHECK_EQ(input.size, output.size);
  DCHECK_EQ(input.type, output.type);

  EmitRexPrefix(output, input);

  if (output.is_physical()) {
    if (output.is_integer())
      EmitOpcode(isa::Opcode::MOV_Gv_Ev);
    else if (output.size == ValueSize::Size32)
      EmitOpcode(isa::Opcode::MOVSS_Vss_Wss);
    else
      EmitOpcode(isa::Opcode::MOVSD_Vsd_Wsd);
  } else {
    if (output.is_integer())
      EmitOpcode(isa::Opcode::MOV_Ev_Gv);
    else if (output.size == ValueSize::Size32)
      EmitOpcode(isa::Opcode::MOVSS_Wss_Vss);
    else
      EmitOpcode(isa::Opcode::MOVSD_Wsd_Vsd);
  }

  EmitModRm(output, input);
}

void InstructionHandlerX64::VisitLiteral(LiteralInstruction* instr) {
  auto const input = instr->input(0);
  auto const output = instr->output(0);
  DCHECK_EQ(input.size, output.size);
  DCHECK_EQ(input.type, output.type);
  DCHECK(output.is_integer()) << "NYI: float " << *instr;

  if (output.is_physical() && output.size == ValueSize::Size64 &&
      Is32BitLiteral(input)) {
    auto const imm32 = Int32ValueOf(input);
    if (imm32 >= 0) {
      // 8B/r mov r32, imm32 : 8B/r imm32
      auto const reg32 = To32bitRegister(output);
      EmitRexPrefix(reg32, input);
      EmitOpcode(isa::Opcode::MOV_Ev_Iz, ToRegister(reg32));
      Emit32(imm32);
      return;
    }
    // sign extended to 64 bits to r/m64
    // C7/0 mov r/m64, imm32 : REX.W+C7 /0 imm32
    EmitRexPrefix(output, input);
    EmitOpcode(isa::Opcode::MOV_Ev_Iz);
    EmitModRm(output, isa::OpcodeExt::MOV_Ev_Iz);
    Emit32(imm32);
    return;
  }

  EmitRexPrefix(output, input);
  if (output.size == ValueSize::Size8)
    EmitOpcode(isa::Opcode::MOV_Eb_Ib);
  else
    EmitOpcode(isa::Opcode::MOV_Ev_Iz);
  EmitModRm(output, isa::OpcodeExt::MOV_Ev_Iz);
  EmitOperand(input);
}

void InstructionHandlerX64::VisitRet(RetInstruction* instr) {
  EmitOpcode(isa::Opcode::RET);
}

}  // namespace

std::unique_ptr<InstructionVisitor> CodeEmitter::NewInstructionHandler(
    CodeBuffer* code_buffer) {
  return std::make_unique<InstructionHandlerX64>(factory_, code_buffer);
}

}  // namespace lir
}  // namespace elang
