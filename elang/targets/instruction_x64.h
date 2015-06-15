// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_TARGETS_INSTRUCTION_X64_H_
#define ELANG_TARGETS_INSTRUCTION_X64_H_

#include <array>
#include <iosfwd>
#include <iterator>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"

namespace elang {
namespace targets {
namespace x64 {

class Operand;
enum class OperandSize;
enum class Register;

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
class Instruction final {
 public:
  class Operands final {
   public:
    class Iterator final
        : public std::iterator<std::forward_iterator_tag, Operand> {
     public:
      Iterator(const Iterator& other);
      ~Iterator();

      Iterator& operator=(const Iterator& other);
      bool operator==(const Iterator& other) const;
      bool operator!=(const Iterator& other) const;

      Operand operator*() const;
      Operand operator->() const;
      Iterator& operator++();

     private:
      friend class Operands;

      Iterator(const Instruction* instruction, size_t position);

      const Instruction* instruction_;
      size_t position_;
    };

    Operands(const Operands& other);
    ~Operands();

    Iterator begin() const;
    Iterator end() const;
    size_t size() const;

   private:
    friend class Instruction;

    explicit Operands(const Instruction* instruction);

    const Instruction* instruction_;
  };

  Instruction(const Instruction& other);
  ~Instruction();

  Instruction& operator=(const Instruction& other);

  uint8_t byte_at(size_t index) const;
  base::StringPiece mnemonic() const;
  uint32_t prefixes() const;
  uint32_t opcode() const { return opcode_; }
  Operands operands() const;
  size_t size() const { return static_cast<size_t>(size_); }

  static Instruction Decode(const void* start, const void* end);
  bool IsValid() const { return opcode_size_ != 0; }

 private:
  enum class RexBit;
  class Decoder;

  Instruction();

  size_t number_of_operands() const;
  size_t opcode_size() const { return static_cast<size_t>(opcode_size_); }
  uint16_t operand16_at(size_t position) const;
  uint32_t operand32_at(size_t position) const;
  uint64_t operand64_at(size_t position) const;
  uint8_t operand8_at(size_t position) const;
  size_t prefix_size() const { return static_cast<size_t>(prefix_size_); }
  uint8_t rex_byte() const;

  Operand OperandEv(OperandSize size) const;
  bool HasOpdSize() const;
  Operand OperandAt(size_t position) const;
  Operand OperandGv(OperandSize size) const;
  Operand OperandIb() const;
  Operand OperandIv() const;
  Operand OperandIz() const;
  Operand OperandReg(Register name) const;
  OperandSize OperandSizeOf() const;
  int Rex(RexBit rex_bit) const;

  std::array<uint8_t, 16> bytes_;
  uint32_t opcode_;
  uint32_t opcode_size_ : 2;  // [1, 3]
  uint32_t prefix_size_ : 3;  // [0, 4]
  uint32_t size_ : 4;         // [1, 15]
};

std::ostream& operator<<(std::ostream& ostream, const Instruction& instruction);

}  // namespace x64
}  // namespace targets
}  // namespace elang

#endif  // ELANG_TARGETS_INSTRUCTION_X64_H_
