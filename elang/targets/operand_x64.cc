// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "elang/targets/operand_x64.h"

#include "base/logging.h"
#include "elang/targets/register_x64.h"

namespace elang {
namespace targets {
namespace x64 {

namespace {

struct PackedAddress {
  unsigned base : 4;
  unsigned index : 4;
  unsigned scale : 3;
  unsigned size : 3;
};

uint32_t PackAddress(const Operand::Address& address) {
  DCHECK_NE(address.base, Register::None);
  union {
    PackedAddress s;
    uint32_t u32;
  } packed;
  packed.u32 = 0;
  packed.s.size = static_cast<unsigned>(RegisterSizeOf(address.base));
  packed.s.base = static_cast<unsigned>(address.base) & 15;
  packed.s.index = static_cast<unsigned>(address.index) & 15;
  packed.s.scale = static_cast<unsigned>(address.scale);
  return packed.u32;
}

Operand::Address UnpackAddress(int32_t detail, int32_t offset) {
  union {
    PackedAddress s;
    uint32_t u32;
  } packed;
  packed.u32 = detail;
  auto const size = static_cast<OperandSize>(packed.s.size);
  Operand::Address address;
  address.base = RegisterOf(size, packed.s.base);
  address.index = RegisterOf(size, packed.s.index);
  address.scale = static_cast<ScaledIndex>(packed.s.scale);
  address.offset = offset;
  return address;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Operand::Address
//
Operand::Address::Address()
    : base(Register::None),
      index(Register::None),
      scale(ScaledIndex::None),
      offset(0) {
}

//////////////////////////////////////////////////////////////////////
//
// Operand
//
Operand::Operand(const Address& address)
    : detail_(PackAddress(address)),
      offset_(address.offset),
      size_(address.size),
      type_(Type::Address) {
}

Operand::Operand(Immediate immediate)
    : detail_(static_cast<int32_t>(immediate.data >> 32)),
      offset_(static_cast<int32_t>(immediate.data)),
      size_(immediate.size),
      type_(Type::Immediate) {
}

Operand::Operand(Offset offset)
    : detail_(static_cast<uint32_t>(offset.value >> 32)),
      offset_(static_cast<uint32_t>(offset.value)),
      size_(offset.size),
      type_(Type::Offset) {
}

Operand::Operand(Register name)
    : detail_(0),
      offset_(static_cast<int>(name)),
      size_(RegisterSizeOf(name)),
      type_(Type::Register) {
}

Operand::Operand(Relative address)
    : detail_(0),
      offset_(address.value),
      size_(address.size),
      type_(Type::Relative) {
}

Operand::Operand(const Operand& other)
    : detail_(other.detail_),
      offset_(other.offset_),
      size_(other.size_),
      type_(other.type_) {
}

Operand::~Operand() {
}

Operand& Operand::operator=(const Operand& other) {
  detail_ = other.detail_;
  offset_ = other.offset_;
  size_ = other.size_;
  type_ = other.type_;
  return *this;
}

std::ostream& operator<<(std::ostream& ostream, const Operand& operand) {
  switch (operand.type()) {
    case Operand::Type::Address: {
      auto const address = UnpackAddress(operand.detail(), operand.offset());
      switch (operand.size()) {
        case OperandSize::Is8:
          ostream << "byte ptr ";
          break;
        case OperandSize::Is16:
          ostream << "word ptr ";
          break;
        case OperandSize::Is32:
          break;
        case OperandSize::Is64:
          ostream << "qword ptr ";
          break;
        case OperandSize::Is128:
          ostream << "dqword ptr ";
          break;
        case OperandSize::Is256:
          ostream << "qqword ptr ";
          break;
      }
      ostream << "[" << address.base;
      switch (address.scale) {
        case ScaledIndex::Is1:
          ostream << "+" << address.index;
          break;
        case ScaledIndex::Is2:
          ostream << "+" << address.index << "*2";
          break;
        case ScaledIndex::Is4:
          ostream << "+" << address.index << "*4";
          break;
        case ScaledIndex::Is8:
          ostream << "+" << address.index << "*8";
          break;
      }
      if (address.offset > 0)
        ostream << "+" << address.offset;
      else if (address.offset < 0)
        ostream << address.offset;
      return ostream << "]";
    }

    case Operand::Type::Immediate:
      if (operand.size() == OperandSize::Is64) {
        auto const i64 =
            (static_cast<int64_t>(operand.detail()) << 32) | operand.offset();
        return ostream << i64;
      }
      return ostream << operand.offset();
    case Operand::Type::Offset: {
      auto const u64 =
          (static_cast<uint64_t>(operand.detail()) << 32) | operand.offset();
      return ostream << "[0x" << std::hex << u64 << "]";
    }
    case Operand::Type::Register:
      return ostream << static_cast<Register>(operand.offset());
    case Operand::Type::Relative:
      if (operand.offset() < 0)
        return ostream << Register::RIP << operand.offset();
      return ostream << Register::RIP << "+" << operand.offset();
  }
  return ostream << "???";
}

}  // namespace x64
}  // namespace targets
}  // namespace elang
