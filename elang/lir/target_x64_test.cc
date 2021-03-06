// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <memory>
#include <sstream>
#include <string>

#include "elang/lir/target_x64.h"
#include "elang/lir/value.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {

using namespace isa;  // NOLINT [build/namespaces]

TEST(LirTargetX64Test, GetArgumentAt) {
  auto const float32_type = Value::Float32Type();
  auto const float64_type = Value::Float32Type();
  auto const int32_type = Value::Int32Type();
  auto const int64_type = Value::Int64Type();

  EXPECT_EQ(Target::RegisterOf(ECX), Target::ArgumentAt(int32_type, 0));
  EXPECT_EQ(Target::RegisterOf(EDX), Target::ArgumentAt(int32_type, 1));
  EXPECT_EQ(Target::RegisterOf(R8D), Target::ArgumentAt(int32_type, 2));
  EXPECT_EQ(Target::RegisterOf(R9D), Target::ArgumentAt(int32_type, 3));
  EXPECT_EQ(Value::Argument(int32_type, 4), Target::ArgumentAt(int32_type, 4));

  EXPECT_EQ(Target::RegisterOf(RCX), Target::ArgumentAt(int64_type, 0));
  EXPECT_EQ(Target::RegisterOf(RDX), Target::ArgumentAt(int64_type, 1));
  EXPECT_EQ(Target::RegisterOf(R8), Target::ArgumentAt(int64_type, 2));
  EXPECT_EQ(Target::RegisterOf(R9), Target::ArgumentAt(int64_type, 3));
  EXPECT_EQ(Value::Argument(int64_type, 4), Target::ArgumentAt(int64_type, 4));
}

TEST(LirTargetX64Test, GetParameterAt) {
  auto const float32_type = Value::Float32Type();
  auto const float64_type = Value::Float32Type();
  auto const int32_type = Value::Int32Type();
  auto const int64_type = Value::Int64Type();

  EXPECT_EQ(Target::RegisterOf(ECX), Target::ParameterAt(int32_type, 0));
  EXPECT_EQ(Target::RegisterOf(EDX), Target::ParameterAt(int32_type, 1));
  EXPECT_EQ(Target::RegisterOf(R8D), Target::ParameterAt(int32_type, 2));
  EXPECT_EQ(Target::RegisterOf(R9D), Target::ParameterAt(int32_type, 3));
  EXPECT_EQ(Value::Parameter(int32_type, 4),
            Target::ParameterAt(int32_type, 4));

  EXPECT_EQ(Target::RegisterOf(RCX), Target::ParameterAt(int64_type, 0));
  EXPECT_EQ(Target::RegisterOf(RDX), Target::ParameterAt(int64_type, 1));
  EXPECT_EQ(Target::RegisterOf(R8), Target::ParameterAt(int64_type, 2));
  EXPECT_EQ(Target::RegisterOf(R9), Target::ParameterAt(int64_type, 3));
  EXPECT_EQ(Value::Parameter(int64_type, 4),
            Target::ParameterAt(int64_type, 4));
}

TEST(LirTargetX64Test, GetReturn) {
  EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size32,
                  Value::Kind::PhysicalRegister, EAX & 15),
            Target::ReturnAt(Value::Int32Type(), 0));

  EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size64,
                  Value::Kind::PhysicalRegister, RAX & 15),
            Target::ReturnAt(Value::Int64Type(), 0));

  EXPECT_EQ(Value(Value::Type::Float, ValueSize::Size32,
                  Value::Kind::PhysicalRegister, XMM0S & 15),
            Target::ReturnAt(Value::Float32Type(), 0));

  EXPECT_EQ(Value(Value::Type::Float, ValueSize::Size64,
                  Value::Kind::PhysicalRegister, XMM0D & 15),
            Target::ReturnAt(Value::Float64Type(), 0));
}

TEST(LirTargetX64Test, GetRegister) {
  std::array<Register, 16> int8_regs{
      AL,
      CL,
      DL,
      BL,
      SPL,
      BPL,
      SIL,
      DIL,
      R8B,
      R9B,
      R10B,
      R11B,
      R12B,
      R13B,
      R14B,
      R15B,
  };

  std::array<Register, 16> int16_regs{
      AX,
      CX,
      DX,
      BX,
      SP,
      BP,
      SI,
      DI,
      R8W,
      R9W,
      R10W,
      R11W,
      R12W,
      R13W,
      R14W,
      R15W,
  };

  std::array<Register, 16> int32_regs{
      EAX,
      ECX,
      EDX,
      EBX,
      ESP,
      EBP,
      ESI,
      EDI,
      R8D,
      R9D,
      R10D,
      R11D,
      R12D,
      R13D,
      R14D,
      R15D,
  };

  std::array<Register, 16> int64_regs{
      RAX,
      RCX,
      RDX,
      RBX,
      RSP,
      RBP,
      RSI,
      RDI,
      R8,
      R9,
      R10,
      R11,
      R12,
      R13,
      R14,
      R15,
  };

  std::array<Register, 16> float32_regs{
      XMM0S,
      XMM1S,
      XMM2S,
      XMM3S,
      XMM4S,
      XMM5S,
      XMM6S,
      XMM7S,
      XMM8S,
      XMM9S,
      XMM10S,
      XMM11S,
      XMM12S,
      XMM13S,
      XMM14S,
      XMM15S,
  };

  std::array<Register, 16> float64_regs{
      XMM0D,
      XMM1D,
      XMM2D,
      XMM3D,
      XMM4D,
      XMM5D,
      XMM6D,
      XMM7D,
      XMM8D,
      XMM9D,
      XMM10D,
      XMM11D,
      XMM12D,
      XMM13D,
      XMM14D,
      XMM15D,
  };

  for (auto const reg : int8_regs) {
    EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size8,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::RegisterOf(reg));
  }

  for (auto const reg : int16_regs) {
    EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size16,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::RegisterOf(reg));
  }

  for (auto const reg : int32_regs) {
    EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size32,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::RegisterOf(reg));
  }

  for (auto const reg : int64_regs) {
    EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size64,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::RegisterOf(reg));
  }

  for (auto const reg : float32_regs) {
    EXPECT_EQ(Value(Value::Type::Float, ValueSize::Size32,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::RegisterOf(reg));
  }

  for (auto const reg : float64_regs) {
    EXPECT_EQ(Value(Value::Type::Float, ValueSize::Size64,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::RegisterOf(reg));
  }
}

TEST(LirTargetX64Test, IsCalleeSavedRegister) {
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(RAX)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::RegisterOf(RBX)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(RCX)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(RDX)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::RegisterOf(RDI)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::RegisterOf(RSI)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(RBP)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(RSP)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(R8)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(R9)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(R10)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::RegisterOf(R11)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::RegisterOf(R12)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::RegisterOf(R13)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::RegisterOf(R14)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::RegisterOf(R15)));
}

TEST(LirTargetX64Test, IsCallerSavedRegister) {
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::RegisterOf(RAX)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(RBX)));
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::RegisterOf(RCX)));
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::RegisterOf(RDX)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(RDI)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(RSI)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(RBP)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(RSP)));
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::RegisterOf(R8)));
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::RegisterOf(R9)));
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::RegisterOf(R10)));
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::RegisterOf(R11)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(R12)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(R13)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(R14)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::RegisterOf(R15)));
}

TEST(LirTargetX64Test, NaturalRegisterOf) {
  EXPECT_EQ(Target::RegisterOf(isa::RAX),
            Target::NaturalRegisterOf(Target::RegisterOf(isa::RAX)));
  EXPECT_EQ(Target::RegisterOf(isa::RAX),
            Target::NaturalRegisterOf(Target::RegisterOf(isa::EAX)));
  EXPECT_EQ(Target::RegisterOf(isa::RBX),
            Target::NaturalRegisterOf(Target::RegisterOf(isa::BX)));
  EXPECT_EQ(Target::RegisterOf(isa::RCX),
            Target::NaturalRegisterOf(Target::RegisterOf(isa::CL)));
  EXPECT_EQ(Target::RegisterOf(isa::XMM0D),
            Target::NaturalRegisterOf(Target::RegisterOf(isa::XMM0D)));
  EXPECT_EQ(Target::RegisterOf(isa::XMM0D),
            Target::NaturalRegisterOf(Target::RegisterOf(isa::XMM0S)));
}

TEST(LirTargetX64Test, IntPtrType) {
  EXPECT_EQ(Value::Int64Type(), Target::IntPtrType());
  EXPECT_EQ(8, Value::SizeOf(Target::IntPtrType()));
}

}  // namespace lir
}  // namespace elang
