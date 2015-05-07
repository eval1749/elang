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
#include "elang/lir/instructions_forward.h"
#include "elang/lir/literals_forward.h"
#include "elang/lir/value.h"

#ifdef ELANG_TARGET_ARCH_X64
#include "elang/lir/instructions_x64_forward.h"
#endif

namespace elang {
class AtomicString;

namespace api {
class MachineCodeBuilder;
class PassController;
}

namespace lir {
enum class ErrorCode;
class ErrorData;
class ErrorSink;
class LiteralMap;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class ELANG_LIR_EXPORT Factory final : public ZoneOwner {
 public:
  explicit Factory(api::PassController* pass_controller);
  ~Factory();

  // Errors reported by pipeline
  const std::vector<ErrorData*>& errors() const;

  // Literal data map
  LiteralMap* literals() const { return literal_map_.get(); }
  api::PassController* pass_controller() const { return pass_controller_; }

  // Add error.
  void AddError(ErrorCode error_code,
                Value value,
                const std::vector<Value>& details);

  // Returns true if successfully generate machine code function for |function|,
  // otherwise returns false.
  bool GenerateMachineCode(api::MachineCodeBuilder* builder,
                           Function* function);

  // Returns |Literal| associated with |index|.
  Literal* GetLiteral(Value value) const;

  // Returns newly created |BasicBlock|.
  BasicBlock* NewBasicBlock();

  // Returns newly created condition.
  Value NewConditional();

  // Returns newly created |Function|.
  Function* NewFunction(const std::vector<Value> parameters);

  // Returns |Literal| object.
  Value NewFloat32Value(float32_t value);
  Value NewFloat64Value(float64_t value);
  Value NewIntValue(Value type, int64_t value);
  Value NewStringValue(AtomicString* atomic_string);
  Value NewStringValue(base::StringPiece16 data);

  // Returns newly allocated virtual register specified by |type|.
  Value NewRegister(Value type);

  // Unique identifiers
  int NextBasicBlockId();
  int NextInstructionId();

// Create Instruction
#define V(Name, ...) Instruction* New##Name##Instruction();
  FOR_EACH_LIR_INSTRUCTION_0_0(V)
#undef V

#define V(Name, ...) Instruction* New##Name##Instruction(Value input);
  FOR_EACH_LIR_INSTRUCTION_0_1(V)
#undef V

#define V(Name, ...)                                              \
  Instruction* New##Name##Instruction(Value input0, Value input1, \
                                      Value input2, Value input3);
  FOR_EACH_LIR_INSTRUCTION_0_4(V)
#undef V

#define V(Name, ...) \
  Instruction* New##Name##Instruction(Value output, Value input);
  FOR_EACH_LIR_INSTRUCTION_1_1(V)
#undef V

#define V(Name, ...) \
  Instruction* New##Name##Instruction(Value output, Value left, Value right);
  FOR_EACH_LIR_INSTRUCTION_1_2(V)
#undef V

#define V(Name, ...)                                              \
  Instruction* New##Name##Instruction(Value output, Value input0, \
                                      Value input1, Value input2);
  FOR_EACH_LIR_INSTRUCTION_1_3(V)
#undef V

#define V(Name, mnemonic, parameters, ...) \
  Instruction* New##Name##Instruction parameters;
  FOR_EACH_LIR_INSTRUCTION_N_N(V)
#undef V

#ifdef ELANG_TARGET_ARCH_X64
  Instruction* NewDivX64Instruction(Value div_output,
                                    Value mod_output,
                                    Value high_left,
                                    Value low_left,
                                    Value right);
  Instruction* NewMulX64Instruction(Value high_output,
                                    Value low_output,
                                    Value left,
                                    Value right);
#endif

 private:
  base::StringPiece16 NewString(base::StringPiece16 string);
  void RegisterLiteral(Literal* literal);

  std::unordered_map<float32_t, Value> float32_map_;
  std::unordered_map<float64_t, Value> float64_map_;
  std::unordered_map<int32_t, Value> int32_map_;
  std::unordered_map<int64_t, Value> int64_map_;
  const std::unique_ptr<LiteralMap> literal_map_;

  // |ErrorSink| needs |LiteralMap|.
  const std::unique_ptr<ErrorSink> error_sink_;

  int last_basic_block_id_;
  int last_instruction_id_;
  int last_condition_id_;
  int last_float_register_id_;
  int last_general_register_id_;
  api::PassController* const pass_controller_;
  std::unordered_map<base::StringPiece16, Value> string_map_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_FACTORY_H_
