// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_
#define ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_

#include <vector>

#include "base/macros.h"
#include "elang/compiler/analyze/type_values_forward.h"

namespace elang {
namespace compiler {
namespace ir {
class Type;
}
namespace ts {

//////////////////////////////////////////////////////////////////////
//
// Evaluator
//
class Evaluator final {
 public:
  explicit Evaluator(Factory* factory);
  ~Evaluator();

  // Returns atomic type if possible, otherwise specify value.
  Value* Evaluate(Value* value);

  // The entry point of |Evaluator|.
  Value* Unify(Value* value1, Value* value2);

 private:
  Factory* factory() const { return factory_; }

  bool Contains(const AndValue* and_value, ir::Type* type);
  bool Contains(const AndValue* and_value, const UnionValue* union_value);
  bool Contains(const UnionValue* union_value, ir::Type* type);

  Value* GetAnyValue();
  Value* GetEmptyValue();
  Value* NewLiteral(ir::Type* type);

  Value* Unify(AndValue* value1, AndValue* value2);
  Value* Unify(AndValue* value1, Value* value2);

  Value* Unify(Literal* value1, AndValue* value2);
  Value* Unify(Literal* value1, UnionValue* value2);
  Value* Unify(Literal* value1, Literal* value2);
  Value* Unify(Literal* value1, Value* value2);

  Value* Unify(UnionValue* value1, AndValue* value2);
  Value* Unify(UnionValue* value1, UnionValue* value2);
  Value* Unify(UnionValue* value1, Value* value2);

  Value* Unify(Variable* value1, Value* value2);
  Value* Unify(Variable* value1, Variable* value2);

  void Union(Variable* variable1, Variable* variable2);

  Factory* factory_;

  DISALLOW_COPY_AND_ASSIGN(Evaluator);
};

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_
