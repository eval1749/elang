// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_FACTORY_USER_H_
#define ELANG_LIR_FACTORY_USER_H_

#include "base/macros.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/instructions_forward.h"
#include "elang/lir/literals_forward.h"

namespace elang {
class AtomicString;
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// FactoryUser
//
class ELANG_LIR_EXPORT FactoryUser {
 protected:
  explicit FactoryUser(Factory* factory);
  ~FactoryUser();

  Factory* factory() const { return factory_; }

  Literal* GetLiteral(Value value);
  Value NewConditional();
  Value NewFloat32Value(float32_t data);
  Value NewFloat64Value(float64_t data);
  Value NewIntValue(Value type, int64_t value);
  Value NewRegister(Value type);
  Value NewStringValue(AtomicString* atomic_string);
  Value NewStringValue(base::StringPiece16 data);

// New instructions
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

 private:
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(FactoryUser);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_FACTORY_USER_H_
