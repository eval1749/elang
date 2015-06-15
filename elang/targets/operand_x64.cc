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

uint64_t PackAddress(const Operand::Address& address) {
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
  return (static_cast<uint64_t>(packed.u32) << 32) | address.offset;
}

Operand::Address UnpackAddress(uint64_t detail) {
  union {
    PackedAddress s;
    uint32_t u32;
  } packed;
  packed.u32 = static_cast<uint32_t>(detail >> 32);
  auto const size = static_cast<OperandSize>(packed.s.size);
  Operand::Address address;
  address.base = RegisterOf(size, packed.s.base);
  address.index = RegisterOf(size, packed.s.index);
  address.scale = static_cast<ScaledIndex>(packed.s.scale);
  address.offset = static_cast<int32_t>(detail);
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
    : detail_(PackAddress(address)), size_(address.size), type_(Type::Address) {
}

Operand::Operand(Immediate immediate)
    : detail_(immediate.data), size_(immediate.size), type_(Type::Immediate) {
}

Operand::Operand(Offset offset)
    : detail_(offset.value), size_(offset.size), type_(Type::Offset) {
}

Operand::Operand(Register name)
    : detail_(static_cast<int>(name)),
      size_(RegisterSizeOf(name)),
      type_(Type::Register) {
}

Operand::Operand(Relative address)
    : detail_(address.value), size_(address.size), type_(Type::Relative) {
}

Operand::Operand(const Operand& other)
    : detail_(other.detail_), size_(other.size_), type_(other.type_) {
}

Operand::~Operand() {
}

Operand& Operand::operator=(const Operand& other) {
  detail_ = other.detail_;
  size_ = other.size_;
  type_ = other.type_;
  return *this;
}

std::ostream& operator<<(std::ostream& ostream, const Operand& operand) {
  switch (operand.type()) {
    case Operand::Type::Address: {
      auto const address = UnpackAddress(operand.detail());
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
      return ostream << operand.detail();
    case Operand::Type::Offset:
      return ostream << "[0x" << std::hex
                     << static_cast<uint64_t>(operand.detail()) << "]";
    case Operand::Type::Register:
      return ostream << static_cast<Register>(operand.detail());
    case Operand::Type::Relative:
      if (operand.detail() < 0)
        return ostream << Register::RIP << operand.detail();
      return ostream << Register::RIP << "+" << operand.detail();
  }
  return ostream << "???";
}

}  // namespace x64
}  // namespace targets
}  // namespace elang
