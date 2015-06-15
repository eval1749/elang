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
  NoOperands,
  One,
  AH,
  AL,
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

#define V1(opcode, opext, mnemonic, format1) \
  Install((opcode << 8) | opext, #mnemonic, OperandFormat::format1);
#define V2(opcode, opext, mnemonic, format1, format2)               \
  Install((opcode << 8) | opext, #mnemonic, OperandFormat::format1, \
          OperandFormat::format2);
  FOR_EACH_X64_OPEXT(V1, V2)
#undef V1
#undef V2
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

void Description::Install(uint32_t opcode, const char* mnemonic) {
  formats_.insert(
      std::make_pair(opcode, Format{Encode(OperandFormat::NoOperands)}));
  mnemonics_.insert(std::make_pair(opcode, mnemonic));
}

base::StringPiece Description::MnemonicOf(uint32_t opcode) const {
  auto const it = mnemonics_.find(opcode);
  return it == mnemonics_.end() ? "" : it->second;
}

OperandFormat Description::OperandFormatOf(uint32_t opcode,
                                           size_t position) const {
  auto const format = FormatOf(opcode);
  auto value = format.operands;
  for (auto counter = 0; counter < position; ++counter)
    value >>= 8;
  value &= 0xFF;
  DCHECK_NE(value, 0u);
  return static_cast<OperandFormat>(value);
}

int ExtractBits(uint8_t byte, int start, int end) {
  DCHECK_LT(start, end);
  DCHECK_GE(start, 0);
  auto const mask = (1 << (end - start)) - 1;
  return static_cast<int>((byte >> start) & mask);
}

int Rex(uint8_t rex, int position) {
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
enum class Instruction::RexBit {
  W = 8,
  R = 4,
  X = 2,
  B = 1,
};

struct Instruction::DecodeResult {
  const uint8_t* start;
  const uint8_t* end;
  uint32_t opcode;
  size_t prefix_size;
  size_t opcode_size;
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
    : Instruction(Decode(static_cast<const uint8_t*>(start),
                         static_cast<const uint8_t*>(end))) {
}

Instruction::Instruction(const DecodeResult& result)
    : opcode_(result.opcode),
      opcode_size_(static_cast<uint32_t>(result.opcode_size)),
      prefix_size_(static_cast<uint32_t>(result.prefix_size)),
      size_(static_cast<uint32_t>(result.end - result.start)) {
  DCHECK_GE(size_, opcode_size_ + prefix_size_);
  ::memcpy(bytes_.data(), result.start, size_);
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
  if (!IsValid())
    return base::StringPiece();
  return Description::Get()->MnemonicOf(opcode());
}

size_t Instruction::number_of_operands() const {
  if (!IsValid())
    return 0;
  auto const format = Description::Get()->FormatOf(opcode());
  if (format.operands == static_cast<uint32_t>(OperandFormat::NoOperands))
    return 0;
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

Instruction::Operands Instruction::operands() const {
  return Operands(this);
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

Instruction::DecodeResult Instruction::Decode(const uint8_t* start,
                                              const uint8_t* end) {
  auto result = DecodeOpcode(start, end);
  DCHECK_LE(result.opcode_size, 3);
  if (!result.opcode_size)
    return result;

  auto has_66 = false;
  auto has_REXW = false;
  auto mandatory = 0;
  for (auto index = 0; index < result.prefix_size; ++index) {
    auto const code = start[index];
    if (code == 0x66)
      has_66 = true;
    else if (code == 0xF2 || code == 0xF3)
      mandatory = code;
    else if (code >= 0x48 && code <= 0x4F)
      has_REXW = true;
  }

  std::vector<uint32_t> candidates;
  auto candidate = result.opcode;
  auto const shift = candidate > 0xFFFF ? 24 : candidate > 0xFF ? 16 : 8;
  // Note: three-byte opcode 0F 38 F0 (CRC32) can take both 66 and F2 prefixes.
  if (shift <= 2 && has_66 && mandatory)
    candidates.push_back(((0x6600 | mandatory) << shift) | candidate);

  if (mandatory)
    candidates.push_back((mandatory << shift) | candidate);

  if (has_66)
    candidates.push_back((0x66 << shift) | candidate);

  candidates.push_back(candidate);

  auto runner = result.start + result.prefix_size + result.opcode_size;
  for (auto const opcode : candidates) {
    auto const format = Description::Get()->FormatOf(opcode);
    auto operands = format.operands;
    if (!operands)
      continue;
    for (auto value = operands; value; value >>= 8) {
      switch (static_cast<OperandFormat>(value & 0xFF)) {
        case OperandFormat::Eb:
        case OperandFormat::Ev:
          runner += SizeFromModRm(*runner);
          break;
        case OperandFormat::Ib:
        case OperandFormat::Jb:
          ++runner;
          break;
        case OperandFormat::Iv:
          if (has_66) {
            runner += 2;
            break;
          }
          if (has_REXW) {
            runner += 8;
            break;
          }
          runner += 4;
          break;
        case OperandFormat::Iw:
          runner += 2;
          break;
        case OperandFormat::Iz:
          if (has_66) {
            runner += 2;
            break;
          }
          runner += 4;
          break;
        case OperandFormat::Jv:
          runner += 4;
          break;
        case OperandFormat::Ob:
        case OperandFormat::Ov:
          runner += 8;
          break;
      }
    }
    if (runner > end) {
      result.end = end;
      result.opcode_size = 0;
      return result;
    }
    result.end = runner;
    result.opcode = opcode;
    return result;
  }

  DCHECK_LE(runner, end);
  result.end = runner;
  result.opcode_size = 0;
  return result;
}

Instruction::DecodeResult Instruction::DecodeOpcode(const uint8_t* start,
                                                    const uint8_t* end) {
  enum class CodeKind {
    Opcode1,
    Opcode2,
    OpExt,
    Prefix,
  };

  static CodeKind code_kinds[256];
  if (code_kinds[0x66] != CodeKind::Prefix) {
    code_kinds[0x0F] = CodeKind::Opcode2;

    code_kinds[0x80] = CodeKind::OpExt;
    code_kinds[0x81] = CodeKind::OpExt;
    code_kinds[0x82] = CodeKind::OpExt;
    code_kinds[0x83] = CodeKind::OpExt;
    code_kinds[0x8F] = CodeKind::OpExt;
    code_kinds[0xC0] = CodeKind::OpExt;
    code_kinds[0xC1] = CodeKind::OpExt;
    code_kinds[0xC6] = CodeKind::OpExt;
    code_kinds[0xC7] = CodeKind::OpExt;
    code_kinds[0xD0] = CodeKind::OpExt;
    code_kinds[0xD1] = CodeKind::OpExt;
    code_kinds[0xD2] = CodeKind::OpExt;
    code_kinds[0xD3] = CodeKind::OpExt;
    code_kinds[0xF6] = CodeKind::OpExt;
    code_kinds[0xF7] = CodeKind::OpExt;
    code_kinds[0xFE] = CodeKind::OpExt;
    code_kinds[0xFF] = CodeKind::OpExt;

    code_kinds[0x2E] = CodeKind::Prefix;  // CS
    code_kinds[0x36] = CodeKind::Prefix;  // SS
    code_kinds[0x3E] = CodeKind::Prefix;  // DS
    code_kinds[0x26] = CodeKind::Prefix;  // ES
    code_kinds[0x64] = CodeKind::Prefix;  // FS
    code_kinds[0x65] = CodeKind::Prefix;  // GS

    code_kinds[0x2E] = CodeKind::Prefix;  // Branch not taken
    code_kinds[0x3E] = CodeKind::Prefix;  // Branch taken

    code_kinds[0x66] = CodeKind::Prefix;  // Operand size
    code_kinds[0x67] = CodeKind::Prefix;  // Address size

    code_kinds[0xF0] = CodeKind::Prefix;  // LOCK
    code_kinds[0xF2] = CodeKind::Prefix;  // REPNE
    code_kinds[0xF3] = CodeKind::Prefix;  // REP

    code_kinds[0x40] = CodeKind::Prefix;  // REX
    code_kinds[0x41] = CodeKind::Prefix;  // REX_B
    code_kinds[0x42] = CodeKind::Prefix;  // REX_X
    code_kinds[0x43] = CodeKind::Prefix;  // REX_XB
    code_kinds[0x44] = CodeKind::Prefix;  // REX_R
    code_kinds[0x45] = CodeKind::Prefix;  // REX_RB
    code_kinds[0x46] = CodeKind::Prefix;  // REX_RX
    code_kinds[0x47] = CodeKind::Prefix;  // REX_RXB
    code_kinds[0x48] = CodeKind::Prefix;  // REX_WB
    code_kinds[0x49] = CodeKind::Prefix;  // REX_WX
    code_kinds[0x4A] = CodeKind::Prefix;  // REX_WR
    code_kinds[0x4B] = CodeKind::Prefix;  // REX_WXB
    code_kinds[0x4C] = CodeKind::Prefix;  // REX_WR
    code_kinds[0x4D] = CodeKind::Prefix;  // REX_WRB
    code_kinds[0x4E] = CodeKind::Prefix;  // REX_WRX
    code_kinds[0x4F] = CodeKind::Prefix;  // REX_WRXB
  }

  DecodeResult result{start, end, 0};
  for (auto runner = start; runner < end; ++runner) {
    auto const code = *runner;
    switch (code_kinds[code]) {
      case CodeKind::Opcode1:
        result.opcode = *runner;
        result.opcode_size = 1;
        result.end = runner + 1;
        return result;

      case CodeKind::Opcode2:
        ++runner;
        result.end = runner;
        if (runner == end)
          return result;
        if (*runner != 0x38) {
          result.opcode = 0x0F00 | *runner;
          result.opcode_size = 2;
          return result;
        }
        ++runner;
        result.end = runner;
        if (runner == end)
          return result;
        result.opcode = 0x0F3800 | *runner;
        result.opcode_size = 3;
        return result;

      case CodeKind::OpExt:
        ++runner;
        result.end = runner;
        if (runner == end)
          return result;
        result.opcode = (code << 8) | ExtractBits(*runner, 3, 6);
        result.opcode_size = 1;
        return result;

      case CodeKind::Prefix:
        ++result.prefix_size;
        break;
      default:
        NOTREACHED();
        break;
    }
  }
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
    case OperandFormat::None:
      return Operand(Operand::Immediate{OperandSize::Is8, bytes_[0]});

    case OperandFormat::One:
      return Operand(Operand::Immediate{OperandSize::Is8, 1});

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
      return OperandEv(OperandSizeOf());

    case OperandFormat::Gb:
      return OperandGv(OperandSize::Is8);
    case OperandFormat::Gv:
      return OperandGv(OperandSizeOf());

    case OperandFormat::Ib:
      return OperandIb();
    case OperandFormat::Iv:
      return OperandIv();
    case OperandFormat::Iw:
      DCHECK_EQ(position, 0);
      return Operand(Operand::Immediate{OperandSize::Is16, operand16_at(0)});
    case OperandFormat::Iz:
      return OperandIz();

    case OperandFormat::Jb:
      return Operand(Operand::Relative{OperandSize::Is0, operand8_at(0)});

    case OperandFormat::Jv:
      return Operand(Operand::Relative{OperandSize::Is0, operand32_at(0)});

    case OperandFormat::Ob:
      return Operand(Operand::Offset{OperandSize::Is8, operand64_at(0)});
    case OperandFormat::Ov:
      return Operand(Operand::Offset{OperandSizeOf(), operand64_at(0)});

    case OperandFormat::eAX:
      return OperandReg(HasOpdSize() ? Register::AX : Register::EAX);

    case OperandFormat::rAX:
      return OperandReg(Register::RAX);
    case OperandFormat::rBP:
      return OperandReg(Register::RBP);
    case OperandFormat::rBX:
      return OperandReg(Register::RBX);
    case OperandFormat::rCX:
      return OperandReg(Register::RCX);
    case OperandFormat::rDI:
      return OperandReg(Register::RDI);
    case OperandFormat::rDX:
      return OperandReg(Register::RDX);
    case OperandFormat::rSI:
      return OperandReg(Register::RSI);
    case OperandFormat::rSP:
      return OperandReg(Register::RSP);
  }
  NOTREACHED();
  return Operand(Register::SS);
}

Operand Instruction::OperandEv(OperandSize size) const {
  auto const modrm = operand8_at(0);
  auto const mod = static_cast<Mod>(modrm & 0xC0);
  auto const rm = static_cast<Rm>(ExtractBits(modrm, 0, 3));
  if (mod == Mod::Reg)
    return Operand(RegisterOf(size, Rex(RexBit::B) | static_cast<int>(rm)));

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
        RegisterOf(OperandSize::Is64, Rex(RexBit::B) | ExtractBits(sib, 0, 3));
    address.index =
        RegisterOf(OperandSize::Is64, Rex(RexBit::X) | ExtractBits(sib, 3, 6));
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
      RegisterOf(OperandSize::Is64, Rex(RexBit::B) | static_cast<int>(rm));
  if (mod == Mod::Disp8) {
    address.offset = operand8_at(1);
  } else if (mod == Mod::Disp32) {
    address.offset = operand32_at(1);
  }
  address.size = size;
  return Operand(address);
}

Operand Instruction::OperandGv(OperandSize size) const {
  auto const modrm = operand8_at(0);
  auto const reg = ExtractBits(modrm, 3, 6);
  return Operand(RegisterOf(size, Rex(RexBit::R) | reg));
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

Operand Instruction::OperandReg(Register name) const {
  auto const size = OperandSizeOf();
  return Operand(
      RegisterOf(size, (static_cast<int>(name) & 7) | Rex(RexBit::R)));
}

OperandSize Instruction::OperandSizeOf() const {
  if (Rex(RexBit::W))
    return OperandSize::Is64;
  if (HasOpdSize())
    return OperandSize::Is16;
  return OperandSize::Is32;
}

int Instruction::Rex(RexBit rex_bit) const {
  return (rex_byte() & static_cast<int>(rex_bit)) != 0 ? 8 : 0;
}

std::ostream& operator<<(std::ostream& ostream,
                         const Instruction& instruction) {
  ostream << instruction.mnemonic();
  auto separator = " ";
  for (auto const operand : instruction.operands()) {
    ostream << separator << operand;
    separator = ", ";
  }
  return ostream;
}

}  // namespace x64
}  // namespace targets
}  // namespace elang
