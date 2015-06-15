// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "elang/targets/operand_x64.h"
#include "elang/targets/register_x64.h"

namespace elang {
namespace targets {
namespace x64 {

class OperandX64Test : public ::testing::Test {
 protected:
  OperandX64Test() = default;
  ~OperandX64Test() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(OperandX64Test);
};

std::string ToString(Operand operand) {
  std::ostringstream ostream;
  ostream << operand;
  return ostream.str();
}

TEST_F(OperandX64Test, Address32) {
  Operand::Address addr;
  addr.base = Register::EDX;
  addr.index = Register::ECX;
  addr.scale = ScaledIndex::Is1;
  EXPECT_EQ("[EDX+ECX]", ToString(Operand(addr)));
}

TEST_F(OperandX64Test, AddressDisp) {
  Operand::Address disp8;
  disp8.base = Register::RDX;
  disp8.offset = 3;
  disp8.size = OperandSize::Is32;
  EXPECT_EQ("[RDX+3]", ToString(Operand(disp8)));

  Operand::Address disp32;
  disp32.base = Register::RDX;
  disp32.offset = 123256;
  disp32.size = OperandSize::Is32;
  EXPECT_EQ("[RDX+123256]", ToString(Operand(disp32)));
}

TEST_F(OperandX64Test, AddressIndex) {
  Operand::Address index1;
  index1.base = Register::RDX;
  index1.index = Register::RCX;
  index1.scale = ScaledIndex::Is1;
  index1.size = OperandSize::Is32;
  EXPECT_EQ("[RDX+RCX]", ToString(Operand(index1)));

  Operand::Address index2;
  index2.base = Register::RDX;
  index2.index = Register::RCX;
  index2.scale = ScaledIndex::Is2;
  index2.size = OperandSize::Is32;
  EXPECT_EQ("[RDX+RCX*2]", ToString(Operand(index2)));

  Operand::Address index4;
  index4.base = Register::RDX;
  index4.index = Register::RCX;
  index4.scale = ScaledIndex::Is4;
  index4.size = OperandSize::Is32;
  EXPECT_EQ("[RDX+RCX*4]", ToString(Operand(index4)));

  Operand::Address index8;
  index8.base = Register::RDX;
  index8.index = Register::RCX;
  index8.scale = ScaledIndex::Is8;
  index8.size = OperandSize::Is32;
  EXPECT_EQ("[RDX+RCX*8]", ToString(Operand(index8)));
}

TEST_F(OperandX64Test, AddressSize) {
  Operand::Address ptr8;
  ptr8.size = OperandSize::Is8;
  ptr8.base = Register::RAX;

  Operand::Address ptr16;
  ptr16.size = OperandSize::Is16;
  ptr16.base = Register::RAX;

  Operand::Address ptr32;
  ptr32.size = OperandSize::Is32;
  ptr32.base = Register::RAX;

  Operand::Address ptr64;
  ptr64.size = OperandSize::Is64;
  ptr64.base = Register::RAX;

  Operand::Address ptr128;
  ptr128.size = OperandSize::Is128;
  ptr128.base = Register::RAX;

  Operand::Address ptr256;
  ptr256.size = OperandSize::Is256;
  ptr256.base = Register::RAX;

  EXPECT_EQ("[RAX]", ToString(Operand(ptr8)));
  EXPECT_EQ(OperandSize::Is8, Operand(ptr8).size());

  EXPECT_EQ("[RAX]", ToString(Operand(ptr16)));
  EXPECT_EQ(OperandSize::Is16, Operand(ptr16).size());

  EXPECT_EQ("[RAX]", ToString(Operand(ptr32)));
  EXPECT_EQ(OperandSize::Is32, Operand(ptr32).size());

  EXPECT_EQ("[RAX]", ToString(Operand(ptr64)));
  EXPECT_EQ(OperandSize::Is64, Operand(ptr64).size());

  EXPECT_EQ("[RAX]", ToString(Operand(ptr128)));
  EXPECT_EQ(OperandSize::Is128, Operand(ptr128).size());

  EXPECT_EQ("[RAX]", ToString(Operand(ptr256)));
  EXPECT_EQ(OperandSize::Is256, Operand(ptr256).size());
}

TEST_F(OperandX64Test, Immediate) {
  Operand::Immediate imm8{OperandSize::Is8, 42};
  Operand::Immediate imm16{OperandSize::Is16, 1234};
  Operand::Immediate imm32{OperandSize::Is32, 123456};
  Operand::Immediate imm64{OperandSize::Is64, 12345678901234};

  EXPECT_EQ("42", ToString(Operand(imm8)));
  EXPECT_EQ("1234", ToString(Operand(imm16)));
  EXPECT_EQ("123456", ToString(Operand(imm32)));
  EXPECT_EQ("12345678901234", ToString(Operand(imm64)));
}

TEST_F(OperandX64Test, Register) {
  EXPECT_EQ("EAX", ToString(Operand(Register::EAX)));
  EXPECT_EQ("RAX", ToString(Operand(Register::RAX)));
}

TEST_F(OperandX64Test, Offset) {
  Operand::Offset offset{OperandSize::Is32, 0x12345678};
  EXPECT_EQ("[0x12345678]", ToString(Operand(offset)));
}

TEST_F(OperandX64Test, Relative) {
  Operand::Relative offset{OperandSize::Is32, -12345};
  EXPECT_EQ("RIP-12345", ToString(Operand(offset)));
}

}  // namespace x64
}  // namespace targets
}  // namespace elang
