// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "elang/lir/emitters/disasm_x64.h"

#include "base/logging.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// X64Disassembled::Instruction::Format
//
enum class X64Disassembled::Instruction::Format {
  Data8,
  Ev_Gv,
  Gv_Ev,
};

X64Disassembled::Instruction::Format kOneByteOpcodeFormat[256];

//////////////////////////////////////////////////////////////////////
//
// X64Disassembled::Instruction
//
X64Disassembled::Instruction::Instruction(const std::vector<uint8_t> bytes,
                                          int opcode,
                                          int modrm,
                                          Format format)
    : bytes_(bytes), format_(format), modrm_(modrm), opcode_(opcode) {
}

X64Disassembled::Instruction::Instruction(const Instruction& other)
    : Instruction(other.bytes_, other.opcode_, other.modrm_, other.format_) {
}

X64Disassembled::Instruction::~Instruction() {
}

X64Disassembled::Instruction& X64Disassembled::Instruction::operator=(
    const Instruction& other) {
  this->bytes_ = other.bytes_;
  this->format_ = other.format_;
  this->modrm_ = other.modrm_;
  this->opcode_ = other.opcode_;
  return *this;
}

//////////////////////////////////////////////////////////////////////
//
// X64Disassembled::Iterator
//
X64Disassembled::Iterator::Iterator(const uint8_t* code_start,
                                    const uint8_t* code_end)
    : code_end_(code_end), code_start_(code_start) {
}

X64Disassembled::Iterator::Iterator(const Iterator& other)
    : Iterator(other.code_start_, other.code_end_) {
}

X64Disassembled::Iterator::~Iterator() {
}

X64Disassembled::Iterator& X64Disassembled::Iterator::operator=(
    const Iterator& other) {
  code_end_ = other.code_end_;
  code_start_ = other.code_start_;
  return *this;
}

X64Disassembled::Iterator& X64Disassembled::Iterator::operator++() {
  DCHECK_LT(code_start_, code_end_);
  code_start_ += Decode().size();
  return *this;
}

X64Disassembled::Instruction X64Disassembled::Iterator::operator*() const {
  DCHECK_LT(code_start_, code_end_);
  return Decode();
}

bool X64Disassembled::Iterator::operator==(const Iterator& other) const {
  return code_start_ == other.code_start_;
}

bool X64Disassembled::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

X64Disassembled::Instruction X64Disassembled::Iterator::Decode() const {
  enum class State { Prefix, TwoByteOpcode, } state = State::Prefix;
  int address_size = 64;
  int operand_size = 32;
  std::vector<uint8_t> bytes;
  for (auto runner = code_start_; runner < code_end_; ++runner) {
    auto const code = *runner;
    bytes.push_back(code);
    switch (state) {
      case State::Prefix:
        switch (code) {
          case 0x0F:
            state = State::TwoByteOpcode;
            break;
          case 0x66:
            operand_size = 16;
            break;
          case 0x67:
            address_size = 16;
            break;
          case 0xF2:
            break;
          case 0xF3:
            break;
          default:
            return Instruction(bytes, code, 0, kOneByteOpcodeFormat[code]);
        }
        break;

      case State::TwoByteOpcode:
        break;

      default:
        NOTREACHED() << "Bad state " << static_cast<int>(state);
    }
  }
  return Instruction(bytes, 0, 0, Instruction::Format::Data8);
}

//////////////////////////////////////////////////////////////////////
//
// X64Disassembled
//
X64Disassembled::X64Disassembled(const uint8_t* code_start, size_t code_size)
    : code_size_(code_size), code_start_(code_start) {
}

X64Disassembled::X64Disassembled(const X64Disassembled& other)
    : X64Disassembled(other.code_start_, other.code_size_) {
}

X64Disassembled::~X64Disassembled() {
}

X64Disassembled::Iterator X64Disassembled::begin() const {
  return Iterator(code_start_, code_start_ + code_size_);
}

X64Disassembled::Iterator X64Disassembled::end() const {
  return Iterator(code_start_ + code_size_, code_start_ + code_size_);
}

//////////////////////////////////////////////////////////////////////
//
// X64Disassembler
//
X64Disassembler::X64Disassembler(const uint8_t* code_start, size_t code_size)
    : code_size_(code_size), code_start_(code_start) {
}

X64Disassembler::~X64Disassembler() {
}

X64Disassembled X64Disassembler::Run() {
  return X64Disassembled(code_start_, code_size_);
}

std::ostream& operator<<(std::ostream& ostream,
                         const X64Disassembled& disasmed) {
  for (auto const instr : disasmed)
    ostream << instr;
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const X64Disassembled::Instruction& instr) {
  return ostream << std::hex << instr.opcode();
}

}  // namespace lir
}  // namespace elang
