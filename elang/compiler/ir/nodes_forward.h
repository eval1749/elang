// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_IR_NODES_FORWARD_H_
#define ELANG_COMPILER_IR_NODES_FORWARD_H_

namespace elang {
namespace compiler {
namespace ir {

#define FOR_EACH_ANALYZED_DATA(V) \
  V(Class) \
  V(Enum) \
  V(Parameter) \
  V(Signature)

#define V(Name) class Name;
FOR_EACH_ANALYZED_DATA(V)
#undef V

class Factory;
class Node;
class Type;
class Value;

}  // namespace ir
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_IR_NODES_FORWARD_H_
