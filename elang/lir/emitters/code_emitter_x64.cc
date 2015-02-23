// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "elang/lir/emitters/code_buffer.h"
#include "elang/lir/emitters/code_buffer_user.h"
#include "elang/lir/emitters/code_emitter.h"
#include "elang/lir/emitters/isa_x64.h"
#include "elang/lir/emitters/opcodes_x64.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/literals.h"
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

int64_t MinusOne64 = static_cast<int64_t>(-1);
int64_t One64 = static_cast<int64_t>(1);

bool Is8Bit(int data) {
  return (data & 255) == data;
}

bool Is32Bit(int64_t data) {
  if (data >= 0)
    return data < (One64 << 32);
  return data > (MinusOne64 << 32);
}

Value To32bitValue(Value value) {
  DCHECK(value.is_64bit()) << value;
  return Value(value.type, ValueSize::Size32, value.kind, value.data);
}

#if 0
isa::Register ToRegister(Value reg) {
  DCHECK(reg.is_physical());
  if (reg.is_float()) {
    if (reg.is_32bit())
      return static_cast<Register>(isa::XMM0S + (reg.data & 15));
    if (reg.is_64bit())
      return static_cast<Register>(isa::XMM0D + (reg.data & 15));
    NOTREACHED() << "unknown size " << reg;
    return static_cast<Register>(0);
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
  NOTREACHED() << "Unknown size: " << reg;
  return static_cast<Register>(0);
}
#endif

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
  void EmitIz(Value output, int imm);
  void EmitModRm(Mod mod, Register reg, Register rm);
  void EmitModRm(Mod mod, Register reg, Rm rm);
  void EmitModRm(Register reg, Value input);
  void EmitModRm(Value output, Value input);
  void EmitOpcode(isa::Opcode opcode);

  // Emit ModRm byte, reg field from |opext|, r/m field from |output|.
  void EmitOpcodeExt(isa::OpcodeExt opext, Value input);

  // Emit opcode adding |delta| value. This function usually used for opcode
  // encodes register name, e.g. |MOV eAX, Iv|.
  void EmitOpcodePlus(isa::Opcode opcode, int delta);
  void EmitOperand(Value value);

  // Emit REX prefix for ModRm reg and rm fields.
  void EmitRexPrefix(Value reg, Value rm);

  // Emit REX prefix for ModRm rm fields.
  void EmitRexPrefix(Value rm);

  // TODO(eval1749) We should have|EmitRexPrefix(reg, base, index)| for
  // REX_X bit.

  // Emit SIB byte.
  void EmitSib(Scale scale, Register index, Register base);

  void HandleIntegerArithmetic(Instruction* instr,
                               isa::Opcode op_eb_gb,
                               isa::OpcodeExt opext);

  void HandleShiftInstruction(Instruction* instr, isa::OpcodeExt opext);

  int32_t Int32ValueOf(Value literal) const;
  int64_t Int64ValueOf(Value literal) const;

  // InstructionVisitor
  void VisitAdd(AddInstruction* instr) final;
  void VisitBitAnd(BitAndInstruction* instr) final;
  void VisitBitOr(BitOrInstruction* instr) final;
  void VisitBitXor(BitXorInstruction* instr) final;
  void VisitCall(CallInstruction* instr) final;
  void VisitCopy(CopyInstruction* instr) final;
  void VisitLiteral(LiteralInstruction* instr) final;
  void VisitRet(RetInstruction* instr) final;
  void VisitShl(ShlInstruction* instr) final;
  void VisitShr(ShrInstruction* instr) final;
  void VisitSub(SubInstruction* instr) final;
  void VisitUShr(UShrInstruction* instr) final;

  const Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(InstructionHandlerX64);
};

InstructionHandlerX64::InstructionHandlerX64(const Factory* factory,
                                             CodeBuffer* code_buffer)
    : CodeBufferUser(code_buffer), factory_(factory) {
}

void InstructionHandlerX64::EmitIz(Value output, int imm) {
  if (output.is_8bit()) {
    Emit8(imm);
    return;
  }
  if (output.is_16bit()) {
    Emit16(imm);
    return;
  }
  Emit32(imm);
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

void InstructionHandlerX64::EmitOpcodeExt(isa::OpcodeExt opext, Value input) {
  EmitModRm(Target::GetRegister(static_cast<Register>(opext)), input);
}

void InstructionHandlerX64::EmitOpcodePlus(isa::Opcode opcode, int delta) {
  EmitOpcode(static_cast<isa::Opcode>(static_cast<int>(opcode) + (delta & 7)));
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
      case ValueSize::Size64:
      case ValueSize::Size32:
        Emit32(value.data);
        return;
    }
  }
  if (value.is_literal()) {
    auto const literal = factory_->GetLiteral(value);
    if (auto const i32 = literal->as<Int32Literal>())
      return Emit32(i32->data());
    if (auto const i64 = literal->as<Int64Literal>()) {
      DCHECK(Is32Bit(i64->data()));
      return Emit32(i64->data());
    }
  }
  AssociateValue(value);
  Emit32(0);
}

void InstructionHandlerX64::EmitRexPrefix(Value rm) {
  if (rm.is_16bit())
    EmitOpcode(isa::Opcode::OPDSIZ);
  auto rex = 0;
  if (rm.is_64bit())
    rex |= isa::REX_W;
  if (rm.is_physical() && rm.data >= 8)
    rex |= isa::REX_B;
  // For access lower 8-bit part of SI, DI, SP, BP
  if (rm.is_8bit() && rm.is_physical() && rm.data >= 4)
    rex |= isa::REX;
  if (!rex)
    return;
  Emit8(isa::REX | rex);
}

void InstructionHandlerX64::EmitRexPrefix(Value reg, Value rm) {
  if (reg.is_16bit())
    EmitOpcode(isa::Opcode::OPDSIZ);
  auto rex = 0;
  if (reg.is_64bit())
    rex |= isa::REX_W;
  if (reg.is_physical() && reg.data >= 8)
    rex |= isa::REX_R;
  if (rm.is_physical() && rm.data >= 8)
    rex |= isa::REX_B;
  // For access lower 8-bit part of SI, DI, SP, BP
  if (reg.is_8bit() && reg.is_physical() && reg.data >= 4)
    rex |= isa::REX;
  // For access lower 8-bit part of SI, DI, SP, BP
  if (rm.is_8bit() && rm.is_physical() && rm.data >= 4)
    rex |= isa::REX;
  if (!rex)
    return;
  Emit8(isa::REX | rex);
}

void InstructionHandlerX64::EmitSib(Scale scale,
                                    Register index,
                                    Register base) {
  Emit8(static_cast<int>(scale) | ((index & 7) << 3) | (base & 7));
}

void InstructionHandlerX64::HandleIntegerArithmetic(Instruction* instr,
                                                    isa::Opcode op_eb_gb,
                                                    isa::OpcodeExt opext) {
  auto const output = instr->output(0);
  auto const input = instr->input(1);
  DCHECK_EQ(output.size, input.size);
  DCHECK_EQ(output.type, input.type);

  if (output.is_8bit()) {
    if (input.is_physical()) {
      // 00 /r: ADD r/m8, r8
      EmitRexPrefix(input, output);
      EmitOpcode(op_eb_gb);
      EmitModRm(input, output);
      return;
    }
    if (input.is_memory_slot()) {
      //  02 /r: ADD r8, r/m8
      EmitRexPrefix(output, input);
      EmitOpcodePlus(op_eb_gb, 2);
      EmitModRm(output, input);
      return;
    }

    auto const imm8 = Int32ValueOf(input);
    if (output.is_physical() && (output.data & 15) == 0) {
      //  04 ib: ADD AL, imm8
      EmitRexPrefix(output);
      EmitOpcodePlus(op_eb_gb, 4);
      Emit8(imm8);
      return;
    }

    // 80 /0 ib: ADD r/m8, imm8
    EmitRexPrefix(output);
    EmitOpcode(isa::Opcode::ADD_Eb_Ib);
    EmitOpcodeExt(opext, output);
    Emit8(imm8);
    return;
  }

  // 16bit, 32bit, 64bit
  if (input.is_physical()) {
    // 01 /r: ADD r/m32, r32
    EmitRexPrefix(input, output);
    EmitOpcodePlus(op_eb_gb, 1);
    EmitModRm(input, output);
    return;
  }

  if (input.is_memory_slot()) {
    // 03 /r: ADD r32, r/m32
    EmitRexPrefix(output, input);
    EmitOpcodePlus(op_eb_gb, 3);
    EmitModRm(output, input);
    return;
  }

  EmitRexPrefix(output);
  auto const imm32 = Int32ValueOf(input);
  if (output.is_physical() && (output.data & 15) == 0) {
    // 05 id: ADD EAX, imm32
    EmitOpcodePlus(op_eb_gb, 5);
    EmitIz(output, imm32);
    return;
  }

  if (Is8Bit(imm32)) {
    // 83 /0 ib: ADD r/m32, imm8
    EmitOpcode(isa::Opcode::ADD_Ev_Ib);
    EmitOpcodeExt(opext, output);
    Emit8(imm32);
    return;
  }

  // 81 /0 id: ADD r/m32, imm32
  EmitOpcode(isa::Opcode::ADD_Ev_Iz);
  EmitOpcodeExt(opext, output);
  EmitIz(output, imm32);
}


// Emit code for Shl, Shl, UShr instrutions.
//
// Opcode extension:
//  SAL/SHL=4, SAR=7, SHR=5
//
// int8:
//  D0 /4    SHL r/m8, 1
//  D2 /4    SHL r/m8, CL
//  C0 /4 ib SHL r/m8, imm8
// int32:
//  D1 /4    SHL r/m32, 1
//  D3 /4    SHL r/m32, CL
//  C1 /4    SHL r/m32, imm8
void InstructionHandlerX64::HandleShiftInstruction(Instruction* instr,
                                                   isa::OpcodeExt opext) {
  auto const count = instr->input(1);
  auto const output = instr->output(0);
  DCHECK_EQ(output, instr->input(0));

  EmitRexPrefix(output);

  if (output.is_8bit()) {
    if (count == Value::SmallInt32(1)) {
      EmitOpcode(isa::Opcode::SHL_Eb_1);
      EmitOpcodeExt(opext, output);
      return;
    }
    if (count == Target::GetRegister(isa::CL)) {
      EmitOpcode(isa::Opcode::SHL_Eb_CL);
      EmitOpcodeExt(opext, output);
      return;
    }
    if (count.is_immediate() && Is8Bit(count.data)) {
      EmitOpcode(isa::Opcode::SHL_Eb_Ib);
      EmitOpcodeExt(opext, output);
      Emit8(count.data);
      return;
    }
    NOTREACHED() << "invalid operand for SHL/SHR: " << *instr;
    return;
  }

  if (count == Value::SmallInt32(1)) {
    EmitOpcode(isa::Opcode::SHL_Ev_1);
    EmitOpcodeExt(opext, output);
    return;
  }
  if (count == Target::GetRegister(isa::CL)) {
    EmitOpcode(isa::Opcode::SHL_Ev_CL);
    EmitOpcodeExt(opext, output);
    return;
  }
  if (count.is_immediate() && Is8Bit(count.data)) {
    EmitOpcode(isa::Opcode::SHL_Ev_Ib);
    EmitOpcodeExt(opext, output);
    Emit8(count.data);
    return;
  }
  NOTREACHED() << "invalid operand for SHL/SHR: " << *instr;
}

int32_t InstructionHandlerX64::Int32ValueOf(Value value) const {
  if (value.is_immediate())
    return value.data;
  DCHECK(value.is_literal()) << value;
  auto const literal = factory_->GetLiteral(value);
  if (auto const i32 = literal->as<Int32Literal>())
    return i32->data();
  if (auto const i64 = literal->as<Int64Literal>()) {
    DCHECK(Is32Bit(i64->data()));
    return static_cast<int32_t>(i64->data());
  }
  NOTREACHED() << value << " isn't 32-bit literal";
  return 0;
}

int64_t InstructionHandlerX64::Int64ValueOf(Value value) const {
  if (value.is_immediate())
    return value.data;
  DCHECK(value.is_literal()) << value;
  auto const literal = factory_->GetLiteral(value);
  if (auto const i32 = literal->as<Int32Literal>())
    return i32->data();
  if (auto const i64 = literal->as<Int64Literal>())
    return i64->data();
  NOTREACHED() << value << " isn't 32-bit literal";
  return 0;
}

// InstructionVisitor

// int8:
//  04 ib           ADD AL, imm8
//  80 /0 ib        ADD r/m8, imm8
//  00 /r           ADD r/m8, r8
//  02 /r           ADD r8, r/m8
//
// int16:
//  66 05 iw        ADD AX, imm16
//  66 81 /0 iw     ADD r/m16, imm16
//  66 83 /0 ib     ADD r/m8, imm8
//  66 01 /r        ADD r/m16, r16
//  66 03 /r        ADD r16, r/m16
//
// int32:
//  05 id           ADD EAX, imm32
//  81 /0 id        ADD r/m32, imm32
//  83 /0 ib        ADD r/m32, imm8
//  01 /r           ADD r/m32, r32
//  03 /r           ADD r32, r/m32
//
// int64:
//  REX.W 05 id     ADD RAX, imm32
//  REX.W 81 /0 id  ADD r/m64, imm32
//  REX.W 83 /0 ib  ADD r/m64, imm8
//  REX.W 01 /r     ADD r/m64, r64
//  REX.W 03 /r     ADD r64, r/m64
//
void InstructionHandlerX64::VisitAdd(AddInstruction* instr) {
  auto const output = instr->output(0);
  DCHECK_EQ(output, instr->input(0)) << *instr;
  if (output.is_integer()) {
    HandleIntegerArithmetic(instr, isa::Opcode::ADD_Eb_Gb,
                            isa::OpcodeExt::ADD_Eb_Ib);
    return;
  }
  NOTREACHED() << "NYI: float add: " << *instr;
}

// Instruction formats are as same as ADD.
// Base opcode = 0x20, opext = 4
void InstructionHandlerX64::VisitBitAnd(BitAndInstruction* instr) {
  auto const output = instr->output(0);
  DCHECK_EQ(output, instr->input(0)) << *instr;
  if (output.is_integer()) {
    HandleIntegerArithmetic(instr, isa::Opcode::AND_Eb_Gb,
                            isa::OpcodeExt::AND_Eb_Ib);
    return;
  }
  NOTREACHED() << "float bitand: " << *instr;
}

// Instruction formats are as same as ADD.
// Base opcode = 0x08, opext = 1
void InstructionHandlerX64::VisitBitOr(BitOrInstruction* instr) {
  auto const output = instr->output(0);
  DCHECK_EQ(output, instr->input(0)) << *instr;
  if (output.is_integer()) {
    HandleIntegerArithmetic(instr, isa::Opcode::OR_Eb_Gb,
                            isa::OpcodeExt::OR_Eb_Ib);
    return;
  }
  NOTREACHED() << "float bitor: " << *instr;
}

// Instruction formats are as same as ADD.
// Base opcode = 0x30, opext = 6
void InstructionHandlerX64::VisitBitXor(BitXorInstruction* instr) {
  auto const output = instr->output(0);
  DCHECK_EQ(output, instr->input(0)) << *instr;
  if (output.is_integer()) {
    HandleIntegerArithmetic(instr, isa::Opcode::XOR_Eb_Gb,
                            isa::OpcodeExt::XOR_Eb_Ib);
    return;
  }
  NOTREACHED() << "float bitxor: " << *instr;
}

void InstructionHandlerX64::VisitCall(CallInstruction* instr) {
  EmitOpcode(isa::Opcode::CALL_Jv);
  EmitOperand(instr->input(0));
}

// int8:
//  88 /r MOV r/m8, r8
//  8A /r MOV r8, r/m8
//
// int16:
//  66 89 /r MOV r/m32, r32
//  66 8B /r MOV r32 r/m32
//
// int32:
//  89 /r MOV r/m32, r32
//  8B /r MOV r32 r/m32
//
// int64:
//  REX.W 89 /r MOV r/m32, r32
//  REX.W 8B /r MOV r32 r/m32
//
// float32:
//  F3 0F 10 /r MOVSS xmm1, xmm2/m32
//  F3 0F 11 /r MOVSS xmm2/m32, xmm
//  VEX.LIG.F3.0F.WIG 10/r VMOVSS xmm1, m32
//  VEX.LIG.F3.0F.WIG 11/r VMOVSS m32, xmm1
//
// float64:
//  F2 0F 10 /r MOVSD xmm1, xmm2/m32
//  F2 0F 11 /r MOVSD xmm2/m32, xmm
//  VEX.LIG.F2.0F.WIG 10/r VMOVSS xmm1, m32
//  VEX.LIG.F2.0F.WIG 11/r VMOVSS m32, xmm1
//
void InstructionHandlerX64::VisitCopy(CopyInstruction* instr) {
  auto const input = instr->input(0);
  auto const output = instr->output(0);
  DCHECK_EQ(input.size, output.size);
  DCHECK_EQ(input.type, output.type);

  EmitRexPrefix(output, input);

  if (output.is_physical()) {
    if (output.is_int8()) {
      EmitOpcode(isa::Opcode::MOV_Gb_Eb);
    } else if (output.is_integer()) {
      EmitOpcode(isa::Opcode::MOV_Gv_Ev);
    } else if (output.is_32bit()) {
      EmitOpcode(isa::Opcode::MOVSS_Vss_Wss);
    } else {
      EmitOpcode(isa::Opcode::MOVSD_Vsd_Wsd);
    }
  } else {
    if (output.is_int8())
      EmitOpcode(isa::Opcode::MOV_Eb_Gb);
    else if (output.is_integer())
      EmitOpcode(isa::Opcode::MOV_Ev_Gv);
    else if (output.is_32bit())
      EmitOpcode(isa::Opcode::MOVSS_Wss_Vss);
    else
      EmitOpcode(isa::Opcode::MOVSD_Wsd_Vsd);
  }

  EmitModRm(output, input);
}

// int8:
//  B0+r imm8   MOV r8, imm8
//  C6 /r imm8  MOV r/m8, imm8
//
// int16:
//  66 B8+r imm8   MOV r16, imm16
//  66 C7 /r imm8  MOV r/m16, imm16
//
// int32:
//  B8+r imm32   MOV r32, imm32
//  C7 /r imm32  MOV r/m32, imm32
//
// int64:
//  B8+r imm32          MOV r32, imm32; imm32 >= 0
//  REX.W B8+r imm64    MOV r64, imm64
//  REX.W C7 0/r imm32  MOV r/m64, imm32; imm32 < 0
//
// Note: imm64 to m64 isn't supported.
// Note: float literal should be lowered to integer literal and 'bitcast'.
//
void InstructionHandlerX64::VisitLiteral(LiteralInstruction* instr) {
  auto const input = instr->input(0);
  auto const output = instr->output(0);
  DCHECK_EQ(input.size, output.size);
  DCHECK_EQ(input.type, output.type);
  DCHECK(output.is_integer()) << "Float literal should be lowered " << *instr;

  if (output.is_64bit()) {
    auto const imm64 = Int64ValueOf(input);
    if (Is32Bit(imm64)) {
      auto const value32 = To32bitValue(output);
      auto const imm32 = static_cast<int32_t>(imm64);
      EmitRexPrefix(Target::GetRegister(isa::EAX), value32);
      if (imm32 >= 0 && output.is_physical()) {
        // B8+r imm32: mov r32, imm32
        EmitOpcodePlus(isa::Opcode::MOV_eAX_Iv, value32.data);
        Emit32(imm32);
        return;
      }

      // sign extended to 64 bits to r/m64
      // REX.W C7 /0 imm32: mov r/m64, imm32
      EmitOpcode(isa::Opcode::MOV_Ev_Iz);
      EmitOpcodeExt(isa::OpcodeExt::MOV_Ev_Iz, output);
      Emit32(imm32);
      return;
    }
    // REX.W B8+r imm64: MOV r64, imm64
    DCHECK(output.is_physical());
    EmitRexPrefix(input, output);
    EmitOpcodePlus(isa::Opcode::MOV_eAX_Iv, output.data);
    Emit64(imm64);
    return;
  }

  EmitRexPrefix(input, output);

  if (output.is_8bit()) {
    if (output.is_physical()) {
      // B0+rb ib: MOV r8, imm8
      EmitOpcodePlus(isa::Opcode::MOV_AL_Ib, output.data);
      EmitOperand(input);
      return;
    }

    // C6/0 Ib: MOV r/m8, imm32
    EmitOpcode(isa::Opcode::MOV_Eb_Ib);
    EmitOpcodeExt(isa::OpcodeExt::MOV_Ev_Iz, output);
    EmitOperand(input);
    return;
  }

  if (output.is_physical()) {
    // B8+r imm32: mov r32, imm32
    EmitOpcodePlus(isa::Opcode::MOV_eAX_Iv, output.data);
    EmitOperand(input);
    return;
  }

  // 66 C7 /0 Iz: MOV r/m16, imm16
  // C7 /0 Iz: MOV r/m32, imm32
  EmitOpcode(isa::Opcode::MOV_Ev_Iz);
  EmitOpcodeExt(isa::OpcodeExt::MOV_Ev_Iz, output);
  EmitOperand(input);
}

void InstructionHandlerX64::VisitRet(RetInstruction* instr) {
  EmitOpcode(isa::Opcode::RET);
}

void InstructionHandlerX64::VisitShl(ShlInstruction* instr) {
  HandleShiftInstruction(instr, isa::OpcodeExt::SHL_Ev_1);
}

void InstructionHandlerX64::VisitShr(ShrInstruction* instr) {
  HandleShiftInstruction(instr, isa::OpcodeExt::SAR_Ev_1);
}

// Instruction formats are as same as ADD.
// Base opcode = 0x28, opext = 5
void InstructionHandlerX64::VisitSub(SubInstruction* instr) {
  auto const output = instr->output(0);
  DCHECK_EQ(output, instr->input(0)) << *instr;
  if (output.is_integer()) {
    HandleIntegerArithmetic(instr, isa::Opcode::SUB_Eb_Gb,
                            isa::OpcodeExt::SUB_Eb_Ib);
    return;
  }
  NOTREACHED() << "NYI: float sub: " << *instr;
}

void InstructionHandlerX64::VisitUShr(UShrInstruction* instr) {
  HandleShiftInstruction(instr, isa::OpcodeExt::SHR_Ev_1);
}

}  // namespace

std::unique_ptr<InstructionVisitor> CodeEmitter::NewInstructionHandler(
    CodeBuffer* code_buffer) {
  return std::make_unique<InstructionHandlerX64>(factory_, code_buffer);
}

}  // namespace lir
}  // namespace elang
