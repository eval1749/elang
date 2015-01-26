// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_TYPE_VALUES_FORWARD_H_
#define ELANG_COMPILER_ANALYSIS_TYPE_VALUES_FORWARD_H_

#include <ostream>

namespace elang {
namespace compiler {
namespace ts {

#define FOR_EACH_ABSTRACT_TYPE_VALUE(V) \
  V(UnionValue)                         \
  V(Value)

#define FOR_EACH_CONCRETE_TYPE_VALUE(V) \
  V(AndValue)                           \
  V(AnyValue)                           \
  V(Argument)                           \
  V(CallValue)                          \
  V(EmptyValue)                         \
  V(InvalidValue)                       \
  V(Literal)                            \
  V(NullValue)                          \
  V(Variable)

#define V(Name) class Name;
FOR_EACH_ABSTRACT_TYPE_VALUE(V)
FOR_EACH_CONCRETE_TYPE_VALUE(V)
#undef V

class Evaluator;
class Factory;

std::ostream& operator<<(std::ostream& ostream, const Value& value);

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_TYPE_VALUES_FORWARD_H_
