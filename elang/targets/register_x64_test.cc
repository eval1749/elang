// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "base/macros.h"
#include "elang/targets/register_x64.h"

namespace elang {
namespace targets {
namespace x64 {

class RegisterX64Test : public ::testing::Test {
 protected:
  RegisterX64Test() = default;
  ~RegisterX64Test() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(RegisterX64Test);
};

std::string ToString(Register reg) {
  std::ostringstream ostream;
  ostream << reg;
  return ostream.str();
}

TEST_F(RegisterX64Test, FloatRegister128) {
  EXPECT_EQ("XMM0", ToString(Register::XMM0));
  EXPECT_EQ("XMM1", ToString(Register::XMM1));
  EXPECT_EQ("XMM2", ToString(Register::XMM2));
  EXPECT_EQ("XMM3", ToString(Register::XMM3));
  EXPECT_EQ("XMM4", ToString(Register::XMM4));
  EXPECT_EQ("XMM5", ToString(Register::XMM5));
  EXPECT_EQ("XMM6", ToString(Register::XMM6));
  EXPECT_EQ("XMM7", ToString(Register::XMM7));
  EXPECT_EQ("XMM8", ToString(Register::XMM8));
  EXPECT_EQ("XMM9", ToString(Register::XMM9));
  EXPECT_EQ("XMM10", ToString(Register::XMM10));
  EXPECT_EQ("XMM11", ToString(Register::XMM11));
  EXPECT_EQ("XMM12", ToString(Register::XMM12));
  EXPECT_EQ("XMM13", ToString(Register::XMM13));
  EXPECT_EQ("XMM14", ToString(Register::XMM14));
  EXPECT_EQ("XMM15", ToString(Register::XMM15));
}

TEST_F(RegisterX64Test, FloatRegister256) {
  EXPECT_EQ("YMM0", ToString(Register::YMM0));
  EXPECT_EQ("YMM1", ToString(Register::YMM1));
  EXPECT_EQ("YMM2", ToString(Register::YMM2));
  EXPECT_EQ("YMM3", ToString(Register::YMM3));
  EXPECT_EQ("YMM4", ToString(Register::YMM4));
  EXPECT_EQ("YMM5", ToString(Register::YMM5));
  EXPECT_EQ("YMM6", ToString(Register::YMM6));
  EXPECT_EQ("YMM7", ToString(Register::YMM7));
  EXPECT_EQ("YMM8", ToString(Register::YMM8));
  EXPECT_EQ("YMM9", ToString(Register::YMM9));
  EXPECT_EQ("YMM10", ToString(Register::YMM10));
  EXPECT_EQ("YMM11", ToString(Register::YMM11));
  EXPECT_EQ("YMM12", ToString(Register::YMM12));
  EXPECT_EQ("YMM13", ToString(Register::YMM13));
  EXPECT_EQ("YMM14", ToString(Register::YMM14));
  EXPECT_EQ("YMM15", ToString(Register::YMM15));
}

TEST_F(RegisterX64Test, Register16) {
  EXPECT_EQ("AX", ToString(Register::AX));
  EXPECT_EQ("BX", ToString(Register::BX));
  EXPECT_EQ("CX", ToString(Register::CX));
  EXPECT_EQ("DX", ToString(Register::DX));
  EXPECT_EQ("DI", ToString(Register::DI));
  EXPECT_EQ("SI", ToString(Register::SI));
  EXPECT_EQ("SP", ToString(Register::SP));
  EXPECT_EQ("R8W", ToString(Register::R8W));
  EXPECT_EQ("R9W", ToString(Register::R9W));
  EXPECT_EQ("R10W", ToString(Register::R10W));
  EXPECT_EQ("R11W", ToString(Register::R11W));
  EXPECT_EQ("R12W", ToString(Register::R12W));
  EXPECT_EQ("R13W", ToString(Register::R13W));
  EXPECT_EQ("R14W", ToString(Register::R14W));
  EXPECT_EQ("R15W", ToString(Register::R15W));
}

TEST_F(RegisterX64Test, Register32) {
  EXPECT_EQ("EAX", ToString(Register::EAX));
  EXPECT_EQ("EBX", ToString(Register::EBX));
  EXPECT_EQ("ECX", ToString(Register::ECX));
  EXPECT_EQ("EDX", ToString(Register::EDX));
  EXPECT_EQ("EDI", ToString(Register::EDI));
  EXPECT_EQ("ESI", ToString(Register::ESI));
  EXPECT_EQ("ESP", ToString(Register::ESP));
  EXPECT_EQ("R8D", ToString(Register::R8D));
  EXPECT_EQ("R9D", ToString(Register::R9D));
  EXPECT_EQ("R10D", ToString(Register::R10D));
  EXPECT_EQ("R11D", ToString(Register::R11D));
  EXPECT_EQ("R12D", ToString(Register::R12D));
  EXPECT_EQ("R13D", ToString(Register::R13D));
  EXPECT_EQ("R14D", ToString(Register::R14D));
  EXPECT_EQ("R15D", ToString(Register::R15D));
}

TEST_F(RegisterX64Test, Register64) {
  EXPECT_EQ("RAX", ToString(Register::RAX));
  EXPECT_EQ("RBX", ToString(Register::RBX));
  EXPECT_EQ("RCX", ToString(Register::RCX));
  EXPECT_EQ("RDX", ToString(Register::RDX));
  EXPECT_EQ("RDI", ToString(Register::RDI));
  EXPECT_EQ("RSI", ToString(Register::RSI));
  EXPECT_EQ("RSP", ToString(Register::RSP));
  EXPECT_EQ("R8", ToString(Register::R8));
  EXPECT_EQ("R9", ToString(Register::R9));
  EXPECT_EQ("R10", ToString(Register::R10));
  EXPECT_EQ("R11", ToString(Register::R11));
  EXPECT_EQ("R12", ToString(Register::R12));
  EXPECT_EQ("R13", ToString(Register::R13));
  EXPECT_EQ("R14", ToString(Register::R14));
  EXPECT_EQ("R15", ToString(Register::R15));

  EXPECT_EQ("RIP", ToString(Register::RIP));
}

TEST_F(RegisterX64Test, Register8) {
  EXPECT_EQ("AL", ToString(Register::AL));
  EXPECT_EQ("BL", ToString(Register::BL));
  EXPECT_EQ("CL", ToString(Register::CL));
  EXPECT_EQ("DL", ToString(Register::DL));
  EXPECT_EQ("DIL", ToString(Register::DIL));
  EXPECT_EQ("SIL", ToString(Register::SIL));
  EXPECT_EQ("SPL", ToString(Register::SPL));
  EXPECT_EQ("R8B", ToString(Register::R8B));
  EXPECT_EQ("R9B", ToString(Register::R9B));
  EXPECT_EQ("R10B", ToString(Register::R10B));
  EXPECT_EQ("R11B", ToString(Register::R11B));
  EXPECT_EQ("R12B", ToString(Register::R12B));
  EXPECT_EQ("R13B", ToString(Register::R13B));
  EXPECT_EQ("R14B", ToString(Register::R14B));
  EXPECT_EQ("R15B", ToString(Register::R15B));

  EXPECT_EQ("AH", ToString(Register::AH));
  EXPECT_EQ("BH", ToString(Register::BH));
  EXPECT_EQ("CH", ToString(Register::CH));
  EXPECT_EQ("DH", ToString(Register::DH));
}

}  // namespace x64
}  // namespace targets
}  // namespace elang
