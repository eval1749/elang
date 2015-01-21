// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_IR_NODES_FORWARD_H_
#define ELANG_COMPILER_IR_NODES_FORWARD_H_

#include <ostream>

namespace elang {
namespace compiler {
namespace ir {

#define FOR_EACH_ABSTRACT_IR_NODE(V) \
  V(Node)                            \
  V(Type)                            \
  V(Value)

#define FOR_EACH_CONCRETE_IR_TYPE_NODE(V) \
  V(Class)                                \
  V(Enum)                                 \
  V(Signature)

#define FOR_EACH_CONCRETE_IR_NODE(V) \
  FOR_EACH_CONCRETE_IR_TYPE_NODE(V)  \
  V(Literal)                         \
  V(Method)                          \
  V(Parameter)                       \
  V(Variable)

#define V(Name) class Name;
FOR_EACH_ABSTRACT_IR_NODE(V)
FOR_EACH_CONCRETE_IR_NODE(V)
#undef V

class Factory;
enum class StorageClass;
class Visitor;

// Print for formatting and debugging.
std::ostream& operator<<(std::ostream& ostream, const Node& node);
std::ostream& operator<<(std::ostream& ostream, StorageClass storage_class);

}  // namespace ir
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_IR_NODES_FORWARD_H_
