// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TARGET_X64_H_
#define ELANG_LIR_TARGET_X64_H_

#include <vector>

#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {
namespace isa {

// Calling convention
// First 4 parameters:
//  RCX, RDX, R8, R9, others passed on stack
//  XMM0, XMM1, XMM2, XMM3

//  HIGH    +--------------+
//  RSP+56  | parameter[6] |
//          +--------------+
//  RSP+48  | parameter[5] |
//          +--------------+
//  RSP+40  | parameter[4] |
//          +--------------+
//  RSP+32  | home[3]      | R9/XMM3
//          +--------------+
//  RSP+24  | home[2]      | R8/XMM2
//          +--------------+
//  RSP+16  | home[1]      | RDX/XMM1
//          +--------------+
//  RSP+8   | home[0]      | RCX/XMM0
//          +--------------+
//  RSP     | return IP    |
//          +--------------+
//  RSP     | callee save  |
//          +--------------+

enum Register {
  // 64-bit register
  RAX = 0x300,
  RCX = 0x301,
  RDX = 0x302,
  RBX = 0x303,
  RSP = 0x304,
  RBP = 0x305,
  RSI = 0x306,
  RDI = 0x307,
  R8 = 0x308,
  R9 = 0x309,
  R10 = 0x30A,
  R11 = 0x30B,
  R12 = 0x30C,
  R13 = 0x30D,
  R14 = 0x30E,
  R15 = 0x30F,

  // 32-bit register
  EAX = 0x200,
  ECX = 0x201,
  EDX = 0x202,
  EBX = 0x203,
  ESP = 0x204,
  EBP = 0x205,
  ESI = 0x206,
  EDI = 0x207,
  R8D = 0x208,
  R9D = 0x209,
  R10D = 0x20A,
  R11D = 0x20B,
  R12D = 0x20C,
  R13D = 0x20D,
  R14D = 0x20E,
  R15D = 0x20F,

  // 16-bit register
  AX = 0x100,
  CX = 0x101,
  DX = 0x102,
  BX = 0x103,
  SP = 0x104,
  BP = 0x105,
  SI = 0x106,
  DI = 0x107,
  R8W = 0x108,
  R9W = 0x109,
  R10W = 0x10A,
  R11W = 0x10B,
  R12W = 0x10C,
  R13W = 0x10D,
  R14W = 0x10E,
  R15W = 0x10F,

  // 8-bit register
  AL = 0x00,
  CL = 0x01,
  DL = 0x02,
  BL = 0x03,
  SPL = 0x04,
  BPL = 0x05,
  SIL = 0x06,
  DIL = 0x07,
  R8L = 0x08,
  R9L = 0x09,
  R10L = 0x0A,
  R11L = 0x0B,
  R12L = 0x0C,
  R13L = 0x0D,
  R14L = 0x0E,
  R15L = 0x0F,

  // Float32 - single precision floating-point
  XMM0S = 0x10,
  XMM1S = 0x11,
  XMM2S = 0x12,
  XMM3S = 0x13,
  XMM4S = 0x14,
  XMM5S = 0x15,
  XMM6S = 0x16,
  XMM7S = 0x17,
  XMM8S = 0x18,
  XMM9S = 0x19,
  XMM10S = 0x1A,
  XMM11S = 0x1B,
  XMM12S = 0x1C,
  XMM13S = 0x1D,
  XMM14S = 0x1E,
  XMM15S = 0x1F,

  // Float64 - double precision floating-point
  XMM0D = 0x20,
  XMM1D = 0x21,
  XMM2D = 0x22,
  XMM3D = 0x23,
  XMM4D = 0x24,
  XMM5D = 0x25,
  XMM6D = 0x26,
  XMM7D = 0x27,
  XMM8D = 0x28,
  XMM9D = 0x29,
  XMM10D = 0x2A,
  XMM11D = 0x2B,
  XMM12D = 0x2C,
  XMM13D = 0x2D,
  XMM14D = 0x2E,
  XMM15D = 0x2F,
};

#define REGISTER_MASK(name) (1 << (name & 15))

const int kNumberOfFloatRegisters = 16;
const int kNumberOfGeneralRegisters = 16;

const int kAllFloatRegisters = (1 << kNumberOfFloatRegisters) - 1;
const int kAllGeneralRegisters = (1 << kNumberOfGeneralRegisters) - 1;

// All registers except for XMM0 are allocatable.
const int kAllocatableFloatRegisters = kAllocatableFloatRegisters;

// All registers except for RBP and RSP are allocatable.
const int kAllocatableGeneralRegisters =
    kAllGeneralRegisters & ~REGISTER_MASK(RBP) & ~REGISTER_MASK(RSP);

// Registers must be saved if callee changes them.
const int kFloatCalleeSavedRegisters =
    REGISTER_MASK(XMM6D) | REGISTER_MASK(XMM7D) | REGISTER_MASK(XMM8D) |
    REGISTER_MASK(XMM9D) | REGISTER_MASK(XMM10D) | REGISTER_MASK(XMM11D) |
    REGISTER_MASK(XMM12D) | REGISTER_MASK(XMM13D) | REGISTER_MASK(XMM14D) |
    REGISTER_MASK(XMM15D);

// Registers must be saved if caller uses them.
const int kFloatCallerSavedRegisters =
    REGISTER_MASK(XMM4D) | REGISTER_MASK(XMM5D);

static_assert(!(kFloatCalleeSavedRegisters & kFloatCallerSavedRegisters),
              "caller and callee registers should not contains same register");

// Registers must be saved if callee changes them. These registers are used to
// hold long-lived values that should be preserved across calls.
const int kGeneralCalleeSavedRegisters =
    REGISTER_MASK(RBX) | REGISTER_MASK(RDI) | REGISTER_MASK(RSI) |
    REGISTER_MASK(R12) | REGISTER_MASK(R13) | REGISTER_MASK(R14) |
    REGISTER_MASK(R15);

// Registers must be saved if caller wants to preserve across call.
// These registers are used to hold temporary value that need not be preserved
// across calls.
const int kGeneralCallerSavedRegisters =
    REGISTER_MASK(R10) | REGISTER_MASK(R11);

static_assert(!(kGeneralCalleeSavedRegisters & kGeneralCallerSavedRegisters),
              "caller and callee registers should not contains same register");

// Parameter registers
const int kGeneralParameterRegisters = REGISTER_MASK(RCX) | REGISTER_MASK(RDX) |
                                       REGISTER_MASK(R8) | REGISTER_MASK(R9);

const int kFloatParameterRegisters =
    REGISTER_MASK(XMM0D) | REGISTER_MASK(XMM1D) | REGISTER_MASK(XMM2D) |
    REGISTER_MASK(XMM3D);

#undef REGISTER_MASK

}  // namespace isa

struct Value;
enum class ValueSize : uint32_t;

class ELANG_LIR_EXPORT Target {
 public:
  Target() = delete;
  ~Target() = delete;

  // TODO(eval1749) |AllocatableXXXRegisters()| should return reference
  // rather than object to avoid copying vector elements.
  static std::vector<Value> AllocatableFloatRegisters();
  static std::vector<Value> AllocatableGeneralRegisters();

  // Returns register or location for argument at |position|.
  static Value GetArgumentAt(Value output, int position);

  // Returns register or location for parameter at |position|.
  static Value GetParameterAt(Value output, int position);

  // Returns physical/pseudo register of |name|.
  static Value GetRegister(isa::Register name);

  // Returns physical register for return value.
  static Value GetReturn(Value type);

  // TODO(eval1749) Make |HasCopyImmediateToMemory()| to take immediate value
  // to check whether immediate value is fit in 32-bit or not.
  static bool HasCopyImmediateToMemory(Value type);
  static bool HasSwapInstruction(Value type);

  // Returns true if |physical| is callee save register.
  static bool IsCalleeSavedRegister(Value physical);

  // Returns true if |physical| is caller save register.
  static bool IsCallerSavedRegister(Value physical);

  // Returns true if |physical| is register parameter
  static bool IsParameterRegister(Value physical);

  // Returns bit size of pointer.
  static ValueSize PointerSize();

  // Returns byte size of pointer.
  static int PointerSizeInByte();
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TARGET_X64_H_
