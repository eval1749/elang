// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_LITERAL_MAP_H_
#define ELANG_LIR_LITERAL_MAP_H_

#include <unordered_map>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class ErrorReporter;
class Factory;
class Instruction;
class Literal;

//////////////////////////////////////////////////////////////////////
//
// LiteralMap maps |Value| to |Literal*|.s
//
class ELANG_LIR_EXPORT LiteralMap {
 public:
  LiteralMap();
  ~LiteralMap();

  Value next_literal_value(Value model) const;

  // Returns |Instruction| associated with |index|.
  Instruction* GetInstruction(Value value) const;

  // Returns |Literal| associated with |index|.
  Literal* GetLiteral(Value value) const;

 private:
  friend class ErrorReporter;
  friend class Factory;

  // Register |instruction| for error message
  Value RegisterInstruction(Instruction* instruction);

  // Register |literal|
  void RegisterLiteral(Literal* literal);

  std::unordered_map<Instruction*, Value> instruction_map_;
  std::vector<Instruction*> instructions_;
  std::vector<Literal*> literals_;

  DISALLOW_COPY_AND_ASSIGN(LiteralMap);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_LITERAL_MAP_H_
