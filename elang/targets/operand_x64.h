// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_TARGETS_OPERAND_X64_H_
#define ELANG_TARGETS_OPERAND_X64_H_

#include <iosfwd>

#include "base/basictypes.h"

namespace elang {
namespace targets {
namespace x64 {

enum class Register;
enum class OperandSize;
enum class ScaledIndex;

//////////////////////////////////////////////////////////////////////
//
// Operand
//
class Operand final {
 public:
  enum class Type {
    Address,
    Immediate,
    Offset,
    Register,
    Relative,
  };

  struct Address {
    OperandSize size;
    Register base;
    Register index;
    ScaledIndex scale;
    int32_t offset;

    Address();
  };

  struct Immediate {
    OperandSize size;
    int64_t data;
  };

  struct Offset {
    OperandSize size;
    uint64_t value;
  };

  struct Relative {
    OperandSize size;
    int value;
  };

  explicit Operand(const Address& address);
  explicit Operand(Immediate immediate);
  explicit Operand(Offset offset);
  explicit Operand(Register name);
  explicit Operand(Relative address);
  Operand(const Operand& other);
  ~Operand();

  int32_t detail() const { return detail_; }
  int32_t offset() const { return offset_; }
  OperandSize size() const { return size_; }
  Type type() const { return type_; }

  Operand& operator=(const Operand& other);

 private:
  int32_t detail_;
  int32_t offset_;
  OperandSize size_;
  Type type_;
};

std::ostream& operator<<(std::ostream& ostream, const Operand& operand);

}  // namespace x64
}  // namespace targets
}  // namespace elang

#endif  // ELANG_TARGETS_OPERAND_X64_H_
