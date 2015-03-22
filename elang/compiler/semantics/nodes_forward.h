// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SEMANTICS_NODES_FORWARD_H_
#define ELANG_COMPILER_SEMANTICS_NODES_FORWARD_H_

#include <ostream>

namespace elang {
namespace compiler {
namespace sm {

#define FOR_EACH_ABSTRACT_SEMANTIC(V) \
  V(Semantic)                         \
  V(Type)                             \
  V(Value)

#define FOR_EACH_CONCRETE_TYPE_SEMANTIC(V) \
  V(ArrayType)                             \
  V(Class)                                 \
  V(Enum)                                  \
  V(Signature)

#define FOR_EACH_CONCRETE_SEMANTIC(V) \
  FOR_EACH_CONCRETE_TYPE_SEMANTIC(V)  \
  V(Literal)                          \
  V(Method)                           \
  V(Parameter)                        \
  V(Variable)

#define V(Name) class Name;
FOR_EACH_ABSTRACT_SEMANTIC(V)
FOR_EACH_CONCRETE_SEMANTIC(V)
#undef V

class Factory;
enum class StorageClass;
class Visitor;

// Print for formatting and debugging.
std::ostream& operator<<(std::ostream& ostream, const Semantic& node);
std::ostream& operator<<(std::ostream& ostream, StorageClass storage_class);

}  // namespace sm
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SEMANTICS_NODES_FORWARD_H_
