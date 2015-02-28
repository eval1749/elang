// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/lir/factory_user.h"

#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// FactoryUser
//
FactoryUser::FactoryUser(Factory* factory) : factory_(factory) {
}

FactoryUser::~FactoryUser() {
}

Literal* FactoryUser::GetLiteral(Value value) {
  return factory()->GetLiteral(value);
}

Value FactoryUser::NewConditional() {
  return factory()->NewConditional();
}

Value FactoryUser::NewFloat32Value(float32_t data) {
  return factory()->NewFloat32Value(data);
}

Value FactoryUser::NewFloat64Value(float64_t data) {
  return factory()->NewFloat64Value(data);
}

Value FactoryUser::NewIntValue(Value type, int64_t value) {
  return factory()->NewIntValue(type, value);
}

Value FactoryUser::NewRegister(Value type) {
  return factory()->NewRegister(type);
}

Value FactoryUser::NewStringValue(base::StringPiece16 data) {
  return factory()->NewStringValue(data);
}

// Creating instructions
Instruction* FactoryUser::NewCmpInstruction(Value output,
                                            IntegerCondition condition,
                                            Value left,
                                            Value right) {
  return factory()->NewCmpInstruction(output, condition, left, right);
}

Instruction* FactoryUser::NewFCmpInstruction(Value output,
                                             FloatCondition condition,
                                             Value left,
                                             Value right) {
  return factory()->NewFCmpInstruction(output, condition, left, right);
}

Instruction* FactoryUser::NewPCopyInstruction(
    const std::vector<Value>& outputs,
    const std::vector<Value>& inputs) {
  return factory()->NewPCopyInstruction(outputs, inputs);
}

#define V(Name, ...)                                   \
  Instruction* FactoryUser::New##Name##Instruction() { \
    return factory()->New##Name##Instruction();        \
  }
FOR_EACH_LIR_INSTRUCTION_0_0(V)
#undef V

#define V(Name, ...)                                              \
  Instruction* FactoryUser::New##Name##Instruction(Value input) { \
    return factory()->New##Name##Instruction(input);              \
  }
FOR_EACH_LIR_INSTRUCTION_0_1(V)
#undef V

#define V(Name, ...)                                               \
  Instruction* FactoryUser::New##Name##Instruction(Value input,    \
                                                   Value input2) { \
    return factory()->New##Name##Instruction(input, input2);       \
  }
FOR_EACH_LIR_INSTRUCTION_0_2(V)
#undef V

#define V(Name, ...)                                              \
  Instruction* FactoryUser::New##Name##Instruction(Value output,  \
                                                   Value input) { \
    return factory()->New##Name##Instruction(output, input);      \
  }
FOR_EACH_LIR_INSTRUCTION_1_1(V)
#undef V

#define V(Name, ...)                                                         \
  Instruction* FactoryUser::New##Name##Instruction(Value output, Value left, \
                                                   Value right) {            \
    return factory()->New##Name##Instruction(output, left, right);           \
  }
FOR_EACH_LIR_INSTRUCTION_1_2(V)
#undef V

}  // namespace lir
}  // namespace elang
