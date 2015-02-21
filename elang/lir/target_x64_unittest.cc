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

  EXPECT_EQ(Target::GetRegister(ECX), Target::GetArgumentAt(int32_type, 0));
  EXPECT_EQ(Target::GetRegister(EDX), Target::GetArgumentAt(int32_type, 1));
  EXPECT_EQ(Target::GetRegister(R8D), Target::GetArgumentAt(int32_type, 2));
  EXPECT_EQ(Target::GetRegister(R9D), Target::GetArgumentAt(int32_type, 3));
  EXPECT_EQ(Value::Argument(Value::Type::Integer, ValueSize::Size32, 4),
            Target::GetArgumentAt(int32_type, 4));

  EXPECT_EQ(Target::GetRegister(RCX), Target::GetArgumentAt(int64_type, 0));
  EXPECT_EQ(Target::GetRegister(RDX), Target::GetArgumentAt(int64_type, 1));
  EXPECT_EQ(Target::GetRegister(R8), Target::GetArgumentAt(int64_type, 2));
  EXPECT_EQ(Target::GetRegister(R9), Target::GetArgumentAt(int64_type, 3));
  EXPECT_EQ(Value::Argument(Value::Type::Integer, ValueSize::Size64, 4),
            Target::GetArgumentAt(int64_type, 4));
}

TEST(LirTargetX64Test, GetParameterAt) {
  auto const float32_type = Value::Float32Type();
  auto const float64_type = Value::Float32Type();
  auto const int32_type = Value::Int32Type();
  auto const int64_type = Value::Int64Type();

  EXPECT_EQ(Target::GetRegister(ECX), Target::GetParameterAt(int32_type, 0));
  EXPECT_EQ(Target::GetRegister(EDX), Target::GetParameterAt(int32_type, 1));
  EXPECT_EQ(Target::GetRegister(R8D), Target::GetParameterAt(int32_type, 2));
  EXPECT_EQ(Target::GetRegister(R9D), Target::GetParameterAt(int32_type, 3));
  EXPECT_EQ(Value::Parameter(Value::Type::Integer, ValueSize::Size32, 4),
            Target::GetParameterAt(int32_type, 4));

  EXPECT_EQ(Target::GetRegister(RCX), Target::GetParameterAt(int64_type, 0));
  EXPECT_EQ(Target::GetRegister(RDX), Target::GetParameterAt(int64_type, 1));
  EXPECT_EQ(Target::GetRegister(R8), Target::GetParameterAt(int64_type, 2));
  EXPECT_EQ(Target::GetRegister(R9), Target::GetParameterAt(int64_type, 3));
  EXPECT_EQ(Value::Parameter(Value::Type::Integer, ValueSize::Size64, 4),
            Target::GetParameterAt(int64_type, 4));
}

TEST(LirTargetX64Test, GetReturn) {
  EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size32,
                  Value::Kind::PhysicalRegister, EAX & 15),
            Target::GetReturn(Value::Int32Type()));

  EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size64,
                  Value::Kind::PhysicalRegister, RAX & 15),
            Target::GetReturn(Value::Int64Type()));

  EXPECT_EQ(Value(Value::Type::Float, ValueSize::Size32,
                  Value::Kind::PhysicalRegister, XMM0S & 15),
            Target::GetReturn(Value::Float32Type()));

  EXPECT_EQ(Value(Value::Type::Float, ValueSize::Size64,
                  Value::Kind::PhysicalRegister, XMM0D & 15),
            Target::GetReturn(Value::Float64Type()));
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
      R8L,
      R9L,
      R10L,
      R11L,
      R12L,
      R13L,
      R14L,
      R15L,
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
              Target::GetRegister(reg));
  }

  for (auto const reg : int16_regs) {
    EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size16,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::GetRegister(reg));
  }

  for (auto const reg : int32_regs) {
    EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size32,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::GetRegister(reg));
  }

  for (auto const reg : int64_regs) {
    EXPECT_EQ(Value(Value::Type::Integer, ValueSize::Size64,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::GetRegister(reg));
  }

  for (auto const reg : float32_regs) {
    EXPECT_EQ(Value(Value::Type::Float, ValueSize::Size32,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::GetRegister(reg));
  }

  for (auto const reg : float64_regs) {
    EXPECT_EQ(Value(Value::Type::Float, ValueSize::Size64,
                    Value::Kind::PhysicalRegister, reg & 15),
              Target::GetRegister(reg));
  }
}

TEST(LirTargetX64Test, IsCalleeSavedRegister) {
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(RAX)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::GetRegister(RBX)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(RCX)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(RDX)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::GetRegister(RDI)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::GetRegister(RSI)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(RBP)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(RSP)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(R8)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(R9)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(R10)));
  EXPECT_FALSE(Target::IsCalleeSavedRegister(Target::GetRegister(R11)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::GetRegister(R12)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::GetRegister(R13)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::GetRegister(R14)));
  EXPECT_TRUE(Target::IsCalleeSavedRegister(Target::GetRegister(R15)));
}

TEST(LirTargetX64Test, IsCallerSavedRegister) {
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(RAX)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(RBX)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(RCX)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(RDX)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(RDI)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(RSI)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(RBP)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(RSP)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(R8)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(R9)));
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::GetRegister(R10)));
  EXPECT_TRUE(Target::IsCallerSavedRegister(Target::GetRegister(R11)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(R12)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(R13)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(R14)));
  EXPECT_FALSE(Target::IsCallerSavedRegister(Target::GetRegister(R15)));
}

TEST(LirTargetX64Test, NaturalRegisterOf) {
  EXPECT_EQ(Target::GetRegister(isa::RAX),
            Target::NaturalRegisterOf(Target::GetRegister(isa::RAX)));
  EXPECT_EQ(Target::GetRegister(isa::RAX),
            Target::NaturalRegisterOf(Target::GetRegister(isa::EAX)));
  EXPECT_EQ(Target::GetRegister(isa::RBX),
            Target::NaturalRegisterOf(Target::GetRegister(isa::BX)));
  EXPECT_EQ(Target::GetRegister(isa::RCX),
            Target::NaturalRegisterOf(Target::GetRegister(isa::CL)));
  EXPECT_EQ(Target::GetRegister(isa::XMM0D),
            Target::NaturalRegisterOf(Target::GetRegister(isa::XMM0D)));
  EXPECT_EQ(Target::GetRegister(isa::XMM0D),
            Target::NaturalRegisterOf(Target::GetRegister(isa::XMM0S)));
}

TEST(LirTargetX64Test, PointerSize) {
  EXPECT_EQ(Value::Int64Type(), Target::IntPtrType());
}

TEST(LirTargetX64Test, PointerSizeInByte) {
  EXPECT_EQ(8, Target::PointerSizeInByte());
}

}  // namespace lir
}  // namespace elang
