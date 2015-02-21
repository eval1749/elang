// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

Value FactoryUser::NewIntValue(Value type, int64_t value) {
  return factory()->NewIntValue(type, value);
}

Value FactoryUser::NewRegister(Value type) {
  return factory()->NewRegister(type);
}

// Creating instructions
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
