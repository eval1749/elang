// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_VALIDATOR_H_
#define ELANG_HIR_VALIDATOR_H_

#include <vector>

#include "base/basictypes.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/instruction_visitor.h"
#include "elang/hir/type_factory_user.h"

namespace elang {
namespace hir {

class BasicBlock;
class DominatorTree;
class Editor;
enum class ErrorCode;
class ErrorData;
class Function;
class Instruction;
class Int32Literal;
class Thing;
class Type;
class TypeFactory;
class Value;

//////////////////////////////////////////////////////////////////////
//
// Validator
//
class ELANG_HIR_EXPORT Validator final : public InstructionVisitor,
                                         public TypeFactoryUser {
 public:
  explicit Validator(Editor* editor);
  ~Validator();

  bool Validate(BasicBlock* basic_block);
  bool Validate(Function* function);
  bool Validate(Instruction* instruction);

 private:
  Editor* editor() const { return editor_; }

  // Returns true if |dominator| dominates |dominatee|.
  bool Dominates(Value* dominator, Instruction* dominatee);

  void Error(ErrorCode error_code, const Value* value);
  void Error(ErrorCode error_code, const Value* value, Thing* detail);
  void Error(ErrorCode error_code,
             const Value* value,
             const std::vector<Thing*>& details);
  void Error(ErrorCode error_code, const Instruction* instruction, int index);
  void Error(ErrorCode error_code,
             const Instruction* instruction,
             int index,
             Thing* detail);

  Value* NewInt32(int32_t data);
  void ValidateArrayAccess(Instruction* instr);

// InstructionVisitor
#define V(Name, ...) void Visit##Name(Name##Instruction* instruction) final;
  FOR_EACH_HIR_INSTRUCTION(V)
#undef V

  DominatorTree* const dominator_tree_;
  Editor* const editor_;
  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(Validator);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_VALIDATOR_H_
