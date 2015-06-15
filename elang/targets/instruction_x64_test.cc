// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "elang/targets/instruction_x64.h"
#include "elang/targets/operand_x64.h"
#include "elang/targets/register_x64.h"

namespace elang {
namespace targets {
namespace x64 {

namespace {
enum class Mod {
  Disp0 = 0,
  Disp8 = 0x40,
  Disp32 = 0x80,
  Reg = 0xC0,
};

enum class Reg {
  AL = 0,
  CL = 1,
  DL = 2,
  BL = 3,
  AH = 4,
  CH = 5,
  DH = 6,
  BH = 7,

  AX = 0,
  CX = 1,
  DX = 2,
  BX = 3,
  SP = 4,
  BP = 5,
  SI = 6,
  DI = 7,

  EAX = 0,
  ECX = 1,
  EDX = 2,
  EBX = 3,
  ESP = 4,
  EBP = 5,
  ESI = 6,
  EDI = 7,

  RAX = 0,
  RCX = 1,
  RDX = 2,
  RBX = 3,
  RSP = 4,
  RBP = 5,
  RSI = 6,
  RDI = 7,
};

uint8_t ModRm(Mod mod, Reg reg, Reg rm) {
  return static_cast<uint8_t>(mod) | (static_cast<uint8_t>(reg) << 3) |
         static_cast<uint8_t>(rm);
}

uint8_t ModRm(Mod mod, int opext, Reg rm) {
  return static_cast<uint8_t>(mod) | (static_cast<uint8_t>(opext) << 3) |
         static_cast<uint8_t>(rm);
}

}  // namespace

class InstructionX64Test : public ::testing::Test {
 protected:
  InstructionX64Test() = default;
  ~InstructionX64Test() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(InstructionX64Test);
};

Instruction InstructionFrom(const std::vector<uint8_t> codes) {
  return Instruction(codes.data(), codes.data() + codes.size());
}

std::string ToString(Instruction instruction) {
  std::ostringstream ostream;
  ostream << instruction;
  return ostream.str();
}

TEST_F(InstructionX64Test, EbGbDisp8) {
  auto const instr =
      InstructionFrom({0x88, ModRm(Mod::Disp8, Reg::DL, Reg::CL), 1});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x88, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(3, instr.size());
  EXPECT_EQ(0x88, instr.byte_at(0));
  EXPECT_EQ(0x51, instr.byte_at(1));
  EXPECT_EQ(1, instr.byte_at(2));
  EXPECT_EQ("MOV [RCX+1], DL", ToString(instr));
}

TEST_F(InstructionX64Test, EbGbReg) {
  auto const instr = InstructionFrom({0x88, ModRm(Mod::Reg, Reg::AL, Reg::CL)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x88, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("MOV CL, AL", ToString(instr));
}

TEST_F(InstructionX64Test, EbGbRegRex) {
  auto const instr =
      InstructionFrom({0x44, 0x88, ModRm(Mod::Reg, Reg::AL, Reg::CL)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x88, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x44, instr.prefixes());
  EXPECT_EQ(3, instr.size());
  EXPECT_EQ("MOV CL, R8B", ToString(instr));
}

TEST_F(InstructionX64Test, EvGvReg16) {
  auto const instr =
      InstructionFrom({0x66, 0x89, ModRm(Mod::Reg, Reg::AL, Reg::CL)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x89, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x66, instr.prefixes());
  EXPECT_EQ(3, instr.size());
  EXPECT_EQ("MOV CX, AX", ToString(instr));
}

TEST_F(InstructionX64Test, EvGvReg16Rex) {
  auto const instr =
      InstructionFrom({0x66, 0x44, 0x89, ModRm(Mod::Reg, Reg::AL, Reg::CL)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x89, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x6644, instr.prefixes());
  EXPECT_EQ(4, instr.size());
  EXPECT_EQ("MOV CX, R8W", ToString(instr));
}

TEST_F(InstructionX64Test, EvGvReg32) {
  auto const instr = InstructionFrom({0x89, ModRm(Mod::Reg, Reg::AL, Reg::CL)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x89, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("MOV ECX, EAX", ToString(instr));
}

TEST_F(InstructionX64Test, EvGvReg64) {
  auto const instr =
      InstructionFrom({0x48, 0x89, ModRm(Mod::Reg, Reg::AL, Reg::CL)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x89, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x48, instr.prefixes());
  EXPECT_EQ(3, instr.size());
  EXPECT_EQ("MOV RCX, RAX", ToString(instr));
}

TEST_F(InstructionX64Test, GvEvReg32) {
  auto const instr = InstructionFrom({0x8B, ModRm(Mod::Reg, Reg::AL, Reg::CL)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x8B, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("MOV EAX, ECX", ToString(instr));
}

TEST_F(InstructionX64Test, GvEvReg32Rex) {
  auto const instr =
      InstructionFrom({0x41, 0x8B, ModRm(Mod::Reg, Reg::AL, Reg::CL)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0x8B, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x41, instr.prefixes());
  EXPECT_EQ(3, instr.size());
  EXPECT_EQ("MOV EAX, R9D", ToString(instr));
}

TEST_F(InstructionX64Test, ImulGvEvIb) {
  auto const instr =
      InstructionFrom({0x6B, ModRm(Mod::Reg, Reg::EBX, Reg::EDX), 1});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("IMUL", instr.mnemonic());
  EXPECT_EQ(0x6B, instr.opcode());
  EXPECT_EQ(3, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(3, instr.size());
  EXPECT_EQ("IMUL EBX, EDX, 1", ToString(instr));
}

TEST_F(InstructionX64Test, ImulGvEvIz) {
  auto const instr =
      InstructionFrom({0x69, ModRm(Mod::Reg, Reg::EBX, Reg::EDX), 1, 2, 3, 4});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("IMUL", instr.mnemonic());
  EXPECT_EQ(0x69, instr.opcode());
  EXPECT_EQ(3, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(6, instr.size());
  EXPECT_EQ("IMUL EBX, EDX, 67305985", ToString(instr));
}

TEST_F(InstructionX64Test, JccJv) {
  auto const instr = InstructionFrom({0x0F, 0x82, 1, 2, 3, 4});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("JB", instr.mnemonic());
  EXPECT_EQ(0x0F82, instr.opcode());
  EXPECT_EQ(1, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(6, instr.size());
  EXPECT_EQ("JB RIP+67305985", ToString(instr));
}

TEST_F(InstructionX64Test, JmpJb) {
  auto const instr = InstructionFrom({0xEB, 2});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("JMP", instr.mnemonic());
  EXPECT_EQ(0xEB, instr.opcode());
  EXPECT_EQ(1, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("JMP RIP+2", ToString(instr));
}

TEST_F(InstructionX64Test, JmpJb2) {
  auto const instr = InstructionFrom({0xEB, 0xFF});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("JMP", instr.mnemonic());
  EXPECT_EQ(0xEB, instr.opcode());
  EXPECT_EQ(1, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("JMP RIP-1", ToString(instr));
}

TEST_F(InstructionX64Test, JmpJv) {
  auto const instr = InstructionFrom({0xE9, 1, 2, 3, 4});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("JMP", instr.mnemonic());
  EXPECT_EQ(0xE9, instr.opcode());
  EXPECT_EQ(1, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(5, instr.size());
  EXPECT_EQ("JMP RIP+67305985", ToString(instr));
}

TEST_F(InstructionX64Test, MovR16Iv) {
  auto const instr = InstructionFrom({0x66, 0xBB, 1, 2});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xBB, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x66, instr.prefixes());
  EXPECT_EQ(4, instr.size());
  EXPECT_EQ("MOV BX, 513", ToString(instr));
}

TEST_F(InstructionX64Test, MovALOb) {
  auto const instr = InstructionFrom({0xA0, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xA0, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(9, instr.size());
  EXPECT_EQ("MOV AL, [0x807060504030201]", ToString(instr));
}

TEST_F(InstructionX64Test, MovAXOv) {
  auto const instr = InstructionFrom({0x66, 0xA1, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xA1, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x66, instr.prefixes());
  EXPECT_EQ(10, instr.size());
  EXPECT_EQ("MOV AX, [0x807060504030201]", ToString(instr));
}

TEST_F(InstructionX64Test, MovEAXOv) {
  auto const instr = InstructionFrom({0xA1, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xA1, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(9, instr.size());
  EXPECT_EQ("MOV EAX, [0x807060504030201]", ToString(instr));
}

TEST_F(InstructionX64Test, MovRAXOv) {
  auto const instr = InstructionFrom({0x48, 0xA1, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xA1, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x48, instr.prefixes());
  EXPECT_EQ(10, instr.size());
  EXPECT_EQ("MOV RAX, [0x807060504030201]", ToString(instr));
}

TEST_F(InstructionX64Test, MovObAL) {
  auto const instr = InstructionFrom({0xA2, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xA2, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(9, instr.size());
  EXPECT_EQ("MOV [0x807060504030201], AL", ToString(instr));
}

TEST_F(InstructionX64Test, MovOvAX) {
  auto const instr = InstructionFrom({0x66, 0xA3, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xA3, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x66, instr.prefixes());
  EXPECT_EQ(10, instr.size());
  EXPECT_EQ("MOV [0x807060504030201], AX", ToString(instr));
}

TEST_F(InstructionX64Test, MovOvEAX) {
  auto const instr = InstructionFrom({0xA3, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xA3, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(9, instr.size());
  EXPECT_EQ("MOV [0x807060504030201], EAX", ToString(instr));
}

TEST_F(InstructionX64Test, MovOvRAX) {
  auto const instr = InstructionFrom({0x48, 0xA3, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xA3, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x48, instr.prefixes());
  EXPECT_EQ(10, instr.size());
  EXPECT_EQ("MOV [0x807060504030201], RAX", ToString(instr));
}
TEST_F(InstructionX64Test, MovR32Iv) {
  auto const instr = InstructionFrom({0xBB, 1, 2, 3, 4});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xBB, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(5, instr.size());
  EXPECT_EQ("MOV EBX, 67305985", ToString(instr));
}

TEST_F(InstructionX64Test, MovR64Iv) {
  auto const instr = InstructionFrom({0x48, 0xBB, 1, 2, 3, 4, 5, 6, 7, 8});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xBB, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x48, instr.prefixes());
  EXPECT_EQ(10, instr.size());
  EXPECT_EQ("MOV RBX, 578437695752307201", ToString(instr));
}

TEST_F(InstructionX64Test, MovR8Ib) {
  auto const instr = InstructionFrom({0xB3, 42});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xB3, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("MOV BL, 42", ToString(instr));
}

TEST_F(InstructionX64Test, MovRm16Iz) {
  auto const instr =
      InstructionFrom({0x66, 0xC7, ModRm(Mod::Reg, 0, Reg::BL), 1, 2});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xC700, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x66, instr.prefixes());
  EXPECT_EQ(5, instr.size());
  EXPECT_EQ("MOV BX, 513", ToString(instr));
}

TEST_F(InstructionX64Test, MovRm32Iz) {
  auto const instr =
      InstructionFrom({0xC7, ModRm(Mod::Reg, 0, Reg::BL), 1, 2, 3, 4});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xC700, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(6, instr.size());
  EXPECT_EQ("MOV EBX, 67305985", ToString(instr));
}

TEST_F(InstructionX64Test, MovRm64Iz) {
  auto const instr =
      InstructionFrom({0x48, 0xC7, ModRm(Mod::Reg, 0, Reg::BL), 1, 2, 3, 4});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xC700, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0x48, instr.prefixes());
  EXPECT_EQ(7, instr.size());
  EXPECT_EQ("MOV RBX, 67305985", ToString(instr));
}

TEST_F(InstructionX64Test, MovRm8Ib) {
  auto const instr = InstructionFrom({0xC6, ModRm(Mod::Reg, 0, Reg::BL), 42});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("MOV", instr.mnemonic());
  EXPECT_EQ(0xC600, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(3, instr.size());
  EXPECT_EQ("MOV BL, 42", ToString(instr));
}

TEST_F(InstructionX64Test, NoOpcode) {
  auto const instr = InstructionFrom({0x66, 0x4E});
  ASSERT_FALSE(instr.IsValid());
  EXPECT_EQ("", ToString(instr));
}

TEST_F(InstructionX64Test, NoOperand) {
  auto const instr = InstructionFrom({0x37});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("AAA", instr.mnemonic());
  EXPECT_EQ(0x37, instr.opcode());
  EXPECT_EQ(0, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(1, instr.size());
  EXPECT_EQ("AAA", ToString(instr));
}

TEST_F(InstructionX64Test, OperandReg) {
  auto const instr = InstructionFrom({0x54});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("PUSH", instr.mnemonic());
  EXPECT_EQ(0x54, instr.opcode());
  EXPECT_EQ(1, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(1, instr.size());
  EXPECT_EQ("PUSH ESP", ToString(instr));
}

TEST_F(InstructionX64Test, OperandRegRexR) {
  auto const instr = InstructionFrom({0x44, 0x54});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("PUSH", instr.mnemonic());
  EXPECT_EQ(0x54, instr.opcode());
  EXPECT_EQ(1, instr.operands().size());
  EXPECT_EQ(0x44, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("PUSH R12D", ToString(instr));
}

TEST_F(InstructionX64Test, OperandRegRexW) {
  auto const instr = InstructionFrom({0x48, 0x54});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("PUSH", instr.mnemonic());
  EXPECT_EQ(0x54, instr.opcode());
  EXPECT_EQ(1, instr.operands().size());
  EXPECT_EQ(0x48, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("PUSH RSP", ToString(instr));
}

TEST_F(InstructionX64Test, OperandRegRexWR) {
  auto const instr = InstructionFrom({0x4C, 0x54});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("PUSH", instr.mnemonic());
  EXPECT_EQ(0x54, instr.opcode());
  EXPECT_EQ(1, instr.operands().size());
  EXPECT_EQ(0x4C, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("PUSH R12", ToString(instr));
}

TEST_F(InstructionX64Test, ShlEvOne) {
  auto const instr = InstructionFrom({0xD1, ModRm(Mod::Reg, 4, Reg::EDI)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("SHL", instr.mnemonic());
  EXPECT_EQ(0xD104, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("SHL EDI, 1", ToString(instr));
}

TEST_F(InstructionX64Test, ShlEvCL) {
  auto const instr = InstructionFrom({0xD3, ModRm(Mod::Reg, 4, Reg::EDI)});
  ASSERT_TRUE(instr.IsValid());
  EXPECT_EQ("SHL", instr.mnemonic());
  EXPECT_EQ(0xD304, instr.opcode());
  EXPECT_EQ(2, instr.operands().size());
  EXPECT_EQ(0, instr.prefixes());
  EXPECT_EQ(2, instr.size());
  EXPECT_EQ("SHL EDI, CL", ToString(instr));
}

}  // namespace x64
}  // namespace targets
}  // namespace elang
