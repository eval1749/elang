// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_DISASM_X64_H_
#define ELANG_LIR_EMITTERS_DISASM_X64_H_

#include <iterator>
#include <ostream>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"

namespace elang {
namespace lir {

class X64Disassembled;

//////////////////////////////////////////////////////////////////////
//
// X64Disassembled
//
class X64Disassembled {
 public:
  class Iterator;

  class Instruction {
   public:
    enum class Format;

    Instruction(const Instruction& other);
    ~Instruction();

    Instruction& operator=(const Instruction& other);

    base::StringPiece mnemonic() const;
    int number_of_operands() const;
    int opcode() const { return opcode_; }
    base::StringPiece operand(int index) const;
    int size() const { return static_cast<int>(bytes_.size()); }

   private:
    friend class Iterator;

    Instruction(const std::vector<uint8_t> bytes,
                int opcode,
                int modrm,
                Format format);

    std::vector<uint8_t> bytes_;
    int opcode_;
    int modrm_;
    Format format_;
  };

  class Iterator {
   public:
    typedef std::forward_iterator_tag iterator_category;
    typedef int difference_type;
    typedef Instruction value_type;
    typedef Instruction* pointer;
    typedef Instruction& reference;

    Iterator(const uint8_t* code_start, const uint8_t* code_size);
    Iterator(const Iterator& other);
    ~Iterator();

    Iterator& operator=(const Iterator& other);
    Iterator& operator++();

    Instruction operator*() const;

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    Instruction Decode() const;

    const uint8_t* code_end_;
    const uint8_t* code_start_;
  };

  X64Disassembled(const X64Disassembled& other);
  ~X64Disassembled();

  X64Disassembled& operator=(const X64Disassembled& other);

  Iterator begin() const;
  Iterator end() const;

 private:
  friend class X64Disassembler;

  X64Disassembled(const uint8_t* code_start, size_t code_size);

  size_t const code_size_;
  const uint8_t* const code_start_;
};

//////////////////////////////////////////////////////////////////////
//
// X64Disassembler
//
class X64Disassembler {
 public:
  X64Disassembler(const uint8_t* code_start, size_t code_size);
  ~X64Disassembler();

  X64Disassembled Run();

 private:
  size_t const code_size_;
  const uint8_t* const code_start_;

  DISALLOW_COPY_AND_ASSIGN(X64Disassembler);
};

std::ostream& operator<<(std::ostream& ostream,
                         const X64Disassembled& disasmed);

std::ostream& operator<<(std::ostream& ostream,
                         const X64Disassembled::Instruction& instruction);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_DISASM_X64_H_
