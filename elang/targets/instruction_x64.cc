// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "elang/targets/instruction_x64.h"

#include "base/logging.h"
#include "elang/targets/instructions_x64.h"
#include "elang/targets/operand_x64.h"
#include "elang/targets/register_x64.h"

namespace elang {
namespace targets {
namespace x64 {

namespace {

enum class Mod {
  Disp0 = 0x00,
  Disp8 = 0x40,
  Disp32 = 0x80,
  Reg = 0xC0,
};

enum class Rm {
  Rm0 = 0,
  Rm1 = 1,
  Rm2 = 2,
  Rm3 = 2,
  Sib = 4,
  Disp32 = 5,
  Rm6 = 6,
  Rm7 = 7,
};

/*
#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "elang/targets/instructions_x64.h"

const char* formats[] = {
#define V0(code, mnemonic)
#define V1(code, mnemonic, o1) #o1,
#define V2(code, mnemonic, o1, o2) V1(code, mnemonic, o1) V1(code, mnemonic, o2)
#define V3(code, mnemonic, o1, o2, o3) \
  V2(code, mnemonic, o1, o2) V1(code, mnemonic, o3)
    FOR_EACH_X64_OPCODE(V0, V1, V2, V3)};

extern "C" void main() {
  std::unordered_set<std::string> set;
  std::vector<std::string> keys;
  for (auto it = std::begin(formats); it < std::end(formats); ++it) {
    std::string key(*it);
    if (set.count(key))
      continue;
    keys.push_back(key);
    set.insert(key);
  }
  std::sort(keys.begin(), keys.end());
  for (auto key : keys)
    std::cout << key << ',' << std::endl;
}
*/

enum class OperandFormat {
  None,
  DB,
  AH,
  AL,
  Ap,
  BH,
  BL,
  CH,
  CL,
  CS,
  DH,
  DL,
  DS,
  DX,
  ES,
  Eb,
  Ed,
  Eq,
  Ev,
  Ew,
  Ey,
  FS,
  GS,
  Gb,
  Gd,
  Gq,
  Gv,
  Gy,
  Ib,
  Iv,
  Iw,
  Iz,
  Jb,
  Jv,
  M,
  Ma,
  Mdq,
  Mp,
  Mq,
  My,
  Nq,
  Ob,
  Ov,
  Pd,
  Pq,
  Qq,
  SS,
  Sw,
  Vdq,
  Vpd,
  Vps,
  Vq,
  Vsd,
  Vss,
  Wdq,
  Wpd,
  Wps,
  Wq,
  Wsd,
  Wss,
  eAX,
  eBP,
  eBX,
  eCX,
  eDI,
  eDX,
  eSI,
  eSP,
  rAX,
  rBP,
  rBX,
  rCX,
  rDI,
  rDX,
  rSI,
  rSP,
};

struct Format {
  uint32_t operands;
};

//////////////////////////////////////////////////////////////////////
//
// Description
//
class Description final {
 public:
  Format FormatOf(uint32_t opcode) const;
  static Description* Get();
  base::StringPiece MnemonicOf(uint32_t opcode) const;
  OperandFormat OperandFormatOf(uint32_t opcode, size_t position) const;

 private:
  Description();
  ~Description();

  static uint32_t Encode(OperandFormat format1,
                         OperandFormat format2,
                         OperandFormat format3);
  static uint32_t Encode(OperandFormat format1, OperandFormat format2);
  static uint32_t Encode(OperandFormat format1);

  void Install(uint32_t opcode,
               const char* mnemonic,
               OperandFormat format1,
               OperandFormat format2,
               OperandFormat format3);
  void Install(uint32_t opcode,
               const char* mnemonic,
               OperandFormat format1,
               OperandFormat format2);
  void Install(uint32_t opcode, const char* mnemonic, OperandFormat format1);
  void Install(uint32_t opcode, const char* mnemonic);

  std::unordered_map<uint32_t, Format> formats_;
  std::unordered_map<uint32_t, base::StringPiece> mnemonics_;

  DISALLOW_COPY_AND_ASSIGN(Description);
};

Description::Description() {
#define V0(opcode, mnemonic) Install(opcode, #mnemonic);
#define V1(opcode, mnemonic, format1) \
  Install(opcode, #mnemonic, OperandFormat::format1);
#define V2(opcode, mnemonic, format1, format2) \
  Install(opcode, #mnemonic, OperandFormat::format1, OperandFormat::format2);
#define V3(opcode, mnemonic, format1, format2, format3)                      \
  Install(opcode, #mnemonic, OperandFormat::format1, OperandFormat::format2, \
          OperandFormat::format3);
  FOR_EACH_X64_OPCODE(V0, V1, V2, V3)
#undef V0
#undef V1
#undef V2
#undef V3
}

Description::~Description() {
}

uint32_t Description::Encode(OperandFormat format1,
                             OperandFormat format2,
                             OperandFormat format3) {
  return Encode(format1, format2) | (Encode(format3) << 16);
}

uint32_t Description::Encode(OperandFormat format1, OperandFormat format2) {
  return Encode(format1) | (Encode(format2) << 8);
}

uint32_t Description::Encode(OperandFormat format1) {
  return static_cast<uint32_t>(format1);
}

Format Description::FormatOf(uint32_t opcode) const {
  auto const it = formats_.find(opcode);
  if (it == formats_.end())
    return Format{0};
  return it->second;
}

Description* Description::Get() {
  static Description* description;
  if (!description)
    description = new Description();
  return description;
}

void Description::Install(uint32_t opcode,
                          const char* mnemonic,
                          OperandFormat format1,
                          OperandFormat format2,
                          OperandFormat format3) {
  formats_.insert(
      std::make_pair(opcode, Format{Encode(format1, format2, format3)}));
  mnemonics_.insert(std::make_pair(opcode, mnemonic));
}

void Description::Install(uint32_t opcode,
                          const char* mnemonic,
                          OperandFormat format1,
                          OperandFormat format2) {
  formats_.insert(std::make_pair(opcode, Format{Encode(format1, format2)}));
  mnemonics_.insert(std::make_pair(opcode, mnemonic));
}

void Description::Install(uint32_t opcode,
                          const char* mnemonic,
                          OperandFormat format1) {
  formats_.insert(std::make_pair(opcode, Format{Encode(format1)}));
  mnemonics_.insert(std::make_pair(opcode, mnemonic));
}

OperandFormat Description::OperandFormatOf(uint32_t opcode,
                                           size_t position) const {
  auto const format = FormatOf(opcode);
  auto value = format.operands;
  for (auto counter = 0; counter < position; ++counter)
    value >>= 8;
  DCHECK_GT(value, 0);
  DCHECK_LE(value, 255);
  return static_cast<OperandFormat>(value);
}

int ExtractBits(uint8_t byte, int start, int end) {
  DCHECK_LT(start, end);
  DCHECK_GE(start, 0);
  auto const mask = (1 << (end - start)) - 1;
  return static_cast<int>((byte >> start) & mask);
}

int RexBit(uint8_t rex, int position) {
  DCHECK_GE(position, 0);
  DCHECK_LE(position, 3);
  return (rex & (1 << position)) << (3 - position);
}

int SizeFromModRm(uint8_t modrm) {
  auto const mod = static_cast<Mod>(modrm & 0xC0);
  auto const rm = static_cast<Rm>(ExtractBits(modrm, 0, 3));
  switch (mod) {
    case Mod::Reg:
      return 1;
    case Mod::Disp0:
      if (rm == Rm::Disp32)
        return 5;
      return rm == Rm::Sib ? 2 : 1;
    case Mod::Disp8:
      return rm == Rm::Sib ? 3 : 2;
    case Mod::Disp32:
      return rm == Rm::Sib ? 6 : 5;
  }
  NOTREACHED();
  return 0;
}

}  // namespace

// Rex prefix:
//  Field   Bits    Definition
//  n/a     7:4     0b0100
//  W       3       0=32-bit, 1=64-bit
//  R       2       Extension of the Mod/Rm reg field
//  X       1       Extension of the Mod/Rm SIB index field
//  B       0       Extension of the Mod/Rm r/m, SIB base or Opcode reg field
enum class Instruction::Rex {
  WRX = 0x4E,
  WRXB = 0x4F,
  WRB = 0x4D,
  WR = 0x4C,
  WXB = 0x4B,
  WX = 0x4A,
  WB = 0x49,
  W = 0x48,
  RXB = 0x47,
  RX = 0x46,
  RB = 0x45,
  R = 0x44,
  X = 0x42,
  XB = 0x43,
  B = 0x41,
  None = 0x40,
};

struct Instruction::DecodeResult {
  std::array<uint8_t, 16> bytes;
  uint32_t opcode;
  size_t prefix_size;
  size_t opcode_size;
  size_t size;
};

//////////////////////////////////////////////////////////////////////
//
// Instruction::Operands
//
Instruction::Operands::Operands(const Instruction* instruction)
    : instruction_(instruction) {
}

Instruction::Operands::Operands(const Operands& other)
    : Operands(other.instruction_) {
}

Instruction::Operands::~Operands() {
}

Instruction::Operands::Iterator Instruction::Operands::begin() const {
  return Iterator(instruction_, 0);
}

Instruction::Operands::Iterator Instruction::Operands::end() const {
  return Iterator(instruction_, size());
}

size_t Instruction::Operands::size() const {
  return instruction_->number_of_operands();
}

//////////////////////////////////////////////////////////////////////
//
// Instruction::Operands::Iterator
//
Instruction::Operands::Iterator::Iterator(const Instruction* instruction,
                                          size_t position)
    : instruction_(instruction), position_(position) {
}

Instruction::Operands::Iterator::Iterator(const Iterator& other)
    : Iterator(other.instruction_, other.position_) {
}

Instruction::Operands::Iterator::~Iterator() {
}

Instruction::Operands::Iterator& Instruction::Operands::Iterator::operator=(
    const Iterator& other) {
  instruction_ = other.instruction_;
  position_ = other.position_;
  return *this;
}

bool Instruction::Operands::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(instruction_, other.instruction_);
  return position_ == other.position_;
}

bool Instruction::Operands::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

Operand Instruction::Operands::Iterator::operator*() const {
  return instruction_->OperandAt(position_);
}

Operand Instruction::Operands::Iterator::operator->() const {
  return instruction_->OperandAt(position_);
}

Instruction::Operands::Iterator& Instruction::Operands::Iterator::operator++() {
  ++position_;
  return *this;
}

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
Instruction::Instruction(const void* start, const void* end)
    : Instruction(Decode(start, end)) {
}

Instruction::Instruction(const DecodeResult& result)
    : opcode_(result.opcode),
      opcode_size_(static_cast<uint32_t>(result.opcode_size)),
      prefix_size_(static_cast<uint32_t>(result.prefix_size)),
      size_(static_cast<uint32_t>(result.size)) {
  DCHECK_GE(size_, opcode_size_ + prefix_size_);
  ::memcpy(bytes_.data(), result.bytes.data(), size_);
}

Instruction::Instruction(const Instruction& other)
    : opcode_(other.opcode_),
      opcode_size_(other.opcode_size_),
      prefix_size_(other.prefix_size_),
      size_(other.size_) {
  DCHECK_GE(size_, opcode_size_ + prefix_size_);
  ::memcpy(bytes_.data(), other.bytes_.data(), size_);
}

Instruction::~Instruction() {
}

Instruction& Instruction::operator=(const Instruction& other) {
  DCHECK_GE(size_, opcode_size_ + prefix_size_);
  opcode_size_ = other.opcode_size_;
  prefix_size_ = other.prefix_size_;
  size_ = other.size_;
  ::memcpy(bytes_.data(), other.bytes_.data(), size_);
  return *this;
}

base::StringPiece Instruction::mnemonic() const {
  return Description::Get()->MnemonicOf(opcode());
}

size_t Instruction::number_of_operands() const {
  auto const format = Description::Get()->FormatOf(opcode());
  if (format.operands > 0xFFFF)
    return 3;
  if (format.operands > 0xFF)
    return 2;
  return 1;
}

uint16_t Instruction::operand16_at(size_t position) const {
  return static_cast<uint16_t>(operand8_at(position) |
                               (operand8_at(position + 1) << 8));
}

uint32_t Instruction::operand32_at(size_t position) const {
  return operand8_at(position) | (operand8_at(position + 1) << 8) |
         (operand8_at(position + 2) << 16) | (operand8_at(position + 3) << 24);
}

uint8_t Instruction::operand8_at(size_t position) const {
  auto const index = prefix_size_ + opcode_size_ + position;
  DCHECK_LT(index, size_);
  return bytes_[index];
}

uint64_t Instruction::operand64_at(size_t position) const {
  return operand32_at(0) | (static_cast<uint64_t>(operand32_at(4)) << 32);
}

uint32_t Instruction::prefixes() const {
  uint32_t prefix = 0;
  for (size_t index = 0; index < prefix_size(); ++index) {
    prefix <<= 8;
    prefix |= bytes_.at(index);
  }
  return prefix;
}

uint8_t Instruction::rex_byte() const {
  if (!prefix_size())
    return 0;
  auto const byte = bytes_[prefix_size() - 1];
  return byte >= 0x40 && byte <= 0x4F ? byte : 0;
}

Operand Instruction::OperandEv(OperandSize size) const {
  auto const modrm = operand8_at(0);
  auto const rex = rex_byte();
  auto const mod = static_cast<Mod>(modrm & 0xC0);
  auto const rm = static_cast<Rm>(ExtractBits(modrm, 0, 3));
  if (mod == Mod::Reg)
    return Operand(RegisterOf(size, RexBit(rex, 2) | static_cast<int>(rm)));

  if (mod == Mod::Disp0 && rm == Rm::Disp32) {
    Operand::Address address;
    address.base = Register::RIP;
    address.offset = operand32_at(1);
    address.size = size;
    return Operand(address);
  }

  if (rm == Rm::Sib) {
    Operand::Address address;
    // SIB = ss iii bbb
    auto const sib = operand8_at(1);
    address.base =
        RegisterOf(OperandSize::Is64, RexBit(rex, 0) | ExtractBits(sib, 0, 3));
    address.index =
        RegisterOf(OperandSize::Is64, RexBit(rex, 1) | ExtractBits(sib, 3, 6));
    address.scale = static_cast<ScaledIndex>(ExtractBits(sib, 6, 7));
    if (mod == Mod::Disp8) {
      address.offset = operand8_at(2);
    } else if (mod == Mod::Disp32) {
      address.offset = operand32_at(2);
    }
    address.size = size;
    return Operand(address);
  }

  Operand::Address address;
  address.base =
      RegisterOf(OperandSize::Is64, RexBit(rex, 0) | static_cast<int>(rm));
  if (mod == Mod::Disp8) {
    address.offset = operand8_at(1);
  } else if (mod == Mod::Disp32) {
    address.offset = operand32_at(1);
  }
  address.size = size;
  return Operand(address);
}

Operand Instruction::OperandEv() const {
  return OperandEv(OperandSizeOf());
}

Instruction::DecodeResult Instruction::Decode(const void* start_in,
                                              const void* end_in) {
  static bool is_prefix[256];
  if (!is_prefix[0x2E]) {
    is_prefix[0x2E] = true;  // CS
    is_prefix[0x36] = true;  // SS
    is_prefix[0x3E] = true;  // DS
    is_prefix[0x26] = true;  // ES
    is_prefix[0x64] = true;  // FS
    is_prefix[0x65] = true;  // GS

    is_prefix[0x2E] = true;  // Branch not taken
    is_prefix[0x3E] = true;  // Branch taken

    is_prefix[0x66] = true;  // Operand size
    is_prefix[0x67] = true;  // Address size

    is_prefix[0xF0] = true;  // LOCK
    is_prefix[0xF2] = true;  // REPNE
    is_prefix[0xF3] = true;  // REP

    is_prefix[0x40] = true;  // REX
    is_prefix[0x41] = true;  // REX_B
    is_prefix[0x42] = true;  // REX_X
    is_prefix[0x43] = true;  // REX_XB
    is_prefix[0x44] = true;  // REX_R
    is_prefix[0x45] = true;  // REX_RB
    is_prefix[0x46] = true;  // REX_RX
    is_prefix[0x47] = true;  // REX_RXB
    is_prefix[0x48] = true;  // REX_WB
    is_prefix[0x49] = true;  // REX_WX
    is_prefix[0x4A] = true;  // REX_WR
    is_prefix[0x4B] = true;  // REX_WXB
    is_prefix[0x4C] = true;  // REX_WR
    is_prefix[0x4D] = true;  // REX_WRB
    is_prefix[0x4E] = true;  // REX_WRX
    is_prefix[0x4F] = true;  // REX_WRXB
  }
  auto const start = static_cast<const uint8_t*>(start_in);
  auto const end = static_cast<const uint8_t*>(end_in);

  DecodeResult result;
  result.opcode = 0;
  result.opcode_size = 0;
  result.prefix_size = 0;
  result.size = 0;

  auto runner = start;
  auto has_66 = false;
  auto has_F2 = false;
  auto has_F3 = false;
  auto has_REXW = false;
  while (runner < end) {
    auto const code = *runner;
    ++result.size;
    ++runner;
    if (!is_prefix[code]) {
      DCHECK_EQ(result.opcode_size, 0);
      if (code != 0x0F) {
        result.opcode_size = 1;
        break;
      }
      if (runner >= end)
        return result;
      result.opcode_size = 2;
      ++result.size;
      ++runner;
      break;
    }
    ++result.prefix_size;
    if (code == 0x66)
      has_66 = true;
    else if (code == 0xF2)
      has_F2 = true;
    else if (code == 0xF3)
      has_F3 = true;
    else if (code >= 0x48 && code <= 0x4F)
      has_REXW = true;
  }

  if (!result.opcode_size) {
    ::memcpy(result.bytes.data(), start,
             std::min(static_cast<size_t>(end - start), result.size));
    return result;
  }

  std::vector<uint32_t> opcodes;
  {
    auto opcode = 0;
    for (auto index = result.prefix_size; index < result.size; ++index)
      opcode = (opcode << 8) | result.bytes[index];

    if (has_66 && has_F2)
      opcodes.push_back(0x66F2 | opcode);
    else if (has_66 && has_F3)
      opcodes.push_back(0x66F3 | opcode);

    if (has_F2)
      opcodes.push_back(0xF2 | opcode);
    else if (has_F3)
      opcodes.push_back(0xF3 | opcode);

    if (has_66)
      opcodes.push_back(0x66 | opcode);
  }

  for (auto const opcode : opcodes) {
    auto const format = Description::Get()->FormatOf(opcode);
    auto operands = format.operands;
    if (!operands)
      continue;
    result.opcode = opcode;
    for (auto value = operands; value; value >>= 8) {
      switch (static_cast<OperandFormat>(value & 0xFF)) {
        case OperandFormat::Eb:
        case OperandFormat::Ev:
          result.size += SizeFromModRm(*runner);
        case OperandFormat::Ib:
          ++result.size;
          break;
        case OperandFormat::Iv:
          if (has_REXW)
            result.size += 8;
          else
            result.size += 4;
          break;
        case OperandFormat::Iw:
          result.size += 2;
          break;
        case OperandFormat::Iz:
          result.size += 4;
          break;
      }
    }
    break;
  }

  ::memcpy(result.bytes.data(), start,
           std::min(static_cast<size_t>(end - start), result.size));
  return result;
}

bool Instruction::HasOpdSize() const {
  for (auto value = prefixes(); value; value >>= 8) {
    if ((value & 0xFF) == 0x66)
      return true;
  }
  return false;
}

Operand Instruction::OperandAt(size_t position) const {
  auto const format = Description::Get()->OperandFormatOf(opcode(), position);
  switch (format) {
    case OperandFormat::DB:
      return Operand(Operand::Immediate{OperandSize::Is8, bytes_[0]});
    case OperandFormat::AH:
      return Operand(Register::AH);
    case OperandFormat::AL:
      return Operand(Register::AL);
    case OperandFormat::BH:
      return Operand(Register::BH);
    case OperandFormat::BL:
      return Operand(Register::BL);
    case OperandFormat::CH:
      return Operand(Register::CH);
    case OperandFormat::CL:
      return Operand(Register::CL);
    case OperandFormat::CS:
      return Operand(Register::CS);
    case OperandFormat::DH:
      return Operand(Register::DH);
    case OperandFormat::DL:
      return Operand(Register::DL);
    case OperandFormat::DS:
      return Operand(Register::DS);
    case OperandFormat::ES:
      return Operand(Register::ES);
    case OperandFormat::FS:
      return Operand(Register::FS);
    case OperandFormat::GS:
      return Operand(Register::GS);

    case OperandFormat::Eb:
      return OperandEv(OperandSize::Is8);
    case OperandFormat::Ev:
    case OperandFormat::M:
      return OperandEv();

    case OperandFormat::Gb:
      return OperandGv(OperandSize::Is8);
    case OperandFormat::Gv:
      return OperandGv(OperandSize::Is8);

    case OperandFormat::Ib:
      return OperandIb();
    case OperandFormat::Iv:
      return OperandIv();
    case OperandFormat::Iz:
      return OperandIz();

    case OperandFormat::eAX:
      return OperandReg(Rex::W, Register::EAX);
    case OperandFormat::eBP:
      return OperandReg(Rex::W, Register::EBP);
    case OperandFormat::eBX:
      return OperandReg(Rex::W, Register::EBX);
    case OperandFormat::eCX:
      return OperandReg(Rex::W, Register::ECX);
    case OperandFormat::eDI:
      return OperandReg(Rex::W, Register::EDI);
    case OperandFormat::eDX:
      return OperandReg(Rex::W, Register::EDX);
    case OperandFormat::eSI:
      return OperandReg(Rex::W, Register::ESI);
    case OperandFormat::eSP:
      return OperandReg(Rex::W, Register::ESP);

    case OperandFormat::rAX:
      return OperandReg(Rex::W, Register::RAX);
    case OperandFormat::rBP:
      return OperandReg(Rex::W, Register::RBP);
    case OperandFormat::rBX:
      return OperandReg(Rex::W, Register::RBX);
    case OperandFormat::rCX:
      return OperandReg(Rex::W, Register::RCX);
    case OperandFormat::rDI:
      return OperandReg(Rex::W, Register::RDI);
    case OperandFormat::rDX:
      return OperandReg(Rex::W, Register::RDX);
    case OperandFormat::rSI:
      return OperandReg(Rex::W, Register::RSI);
    case OperandFormat::rSP:
      return OperandReg(Rex::W, Register::RSP);
  }
  NOTREACHED();
  return Operand(Register::SS);
}

Operand Instruction::OperandGv(OperandSize size) const {
  auto const modrm = operand8_at(0);
  return Operand(RegisterOf(size, ExtractBits(modrm, 4, 7)));
}

Operand Instruction::OperandGv() const {
  return OperandGv(OperandSizeOf());
}

Operand Instruction::OperandIb() const {
  auto const position = size() - opcode_size_ - prefix_size_;
  return Operand(
      Operand::Immediate{OperandSize::Is8, operand8_at(position - 1)});
}

Operand Instruction::OperandIv() const {
  auto const position = size() - opcode_size_ - prefix_size_;
  auto const size = OperandSizeOf();
  switch (size) {
    case OperandSize::Is16:
      return Operand(Operand::Immediate{size, operand16_at(position - 2)});
    case OperandSize::Is32:
      return Operand(Operand::Immediate{size, operand32_at(position - 4)});
    case OperandSize::Is64:
      return Operand(Operand::Immediate{size, operand64_at(position - 8)});
  }
  NOTREACHED();
  return Operand(Register::SS);
}

Operand Instruction::OperandIz() const {
  auto const position = size() - opcode_size_ - prefix_size_;
  auto const size = OperandSizeOf();
  switch (size) {
    case OperandSize::Is16:
      return Operand(Operand::Immediate{size, operand16_at(position - 2)});
    case OperandSize::Is32:
    case OperandSize::Is64:
      return Operand(Operand::Immediate{size, operand32_at(position - 4)});
  }
  NOTREACHED();
  return Operand(Register::SS);
}

Operand Instruction::OperandReg(Rex rex, Register name) const {
  auto const size = OperandSizeOf();
  if ((rex_byte() & static_cast<int>(rex)) == static_cast<int>(rex))
    return Operand(RegisterOf(size, (static_cast<int>(name) & 7) | 8));
  return Operand(name);
}

OperandSize Instruction::OperandSizeOf() const {
  auto const rex = rex_byte();
  if (ExtractBits(rex, 3, 4))
    return OperandSize::Is64;
  if (HasOpdSize())
    return OperandSize::Is16;
  return OperandSize::Is32;
}

}  // namespace x64
}  // namespace targets
}  // namespace elang
