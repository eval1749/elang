// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_FACTORY_H_
#define ELANG_LIR_FACTORY_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/literals_forward.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {
class BasicBlock;
class Function;
class Literal;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class ELANG_LIR_EXPORT Factory final : public ZoneOwner {
 public:
  Factory();
  ~Factory();

  // Returns |Literal| associated with |index|.
  Literal* GetLiteral(Value value);

  // Returns mnemonic string for |opcode|.
  base::StringPiece GetMnemonic(const Instruction* instruction);

  // Returns newly created |BasicBlock|.
  BasicBlock* NewBasicBlock();

  // Returns newly created |Function|.
  Function* NewFunction();

  // Returns |Literal| object.
  Value NewFloat32Value(float32_t value);
  Value NewFloat64Value(float64_t value);
  Value NewInt32Value(int32_t value);
  Value NewInt64Value(int64_t value);
  Value NewStringValue(base::StringPiece16 data);

// Create Instruction
#define V(Name, ...) Name##Instruction* New##Name##Instruction();
  FOR_EACH_LIR_INSTRUCTION(V)
#undef V

  // Unique identifiers
  int NextBasicBlockId();
  int NextInstructionId();

 private:
  Value next_literal_value() const;

  base::StringPiece16 NewString(base::StringPiece16 string);
  Value RegisterLiteral(Literal* literal);

  std::unordered_map<float32_t, Value> float32_map_;
  std::unordered_map<float64_t, Value> float64_map_;
  std::unordered_map<int32_t, Value> int32_map_;
  std::unordered_map<int64_t, Value> int64_map_;
  int last_basic_block_id_;
  int last_instruction_id_;
  std::unordered_map<base::StringPiece16, Value> string_map_;
  std::vector<Literal*> literals_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_FACTORY_H_
