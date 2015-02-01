// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_LITERALS_FORWARD_H_
#define ELANG_LIR_LITERALS_FORWARD_H_

#include <ostream>
#include <string>

#include "elang/base/float_types.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

// Forward declarations
class Editor;
class Factory;
class Literal;
class LiteralVisitor;

// Visitor |V| takes three parameters:
//  Name     capitalized name for C++ class
//  name     small case name
//  cpp_type in C++.
#define FOR_EACH_LIR_SIMPLE_LITERAL(V) \
  V(Float32, float32, float32_t)       \
  V(Float64, float64, float64_t)       \
  V(Int32, int32, int32_t)             \
  V(Int64, int64, int64_t)

#define V(Name, ...) class Name##Literal;
FOR_EACH_LIR_SIMPLE_LITERAL(V)
#undef V

// List of concrete C++ classes representing HIR values other than
// instruction.
#define FOR_EACH_LIR_LITERAL(V) \
  V(BasicBlock)                 \
  V(Float32Literal)             \
  V(Float64Literal)             \
  V(Function)                   \
  V(Int32Literal)               \
  V(Int64Literal)               \
  V(StringLiteral)

#define V(Name, ...) class Name;
FOR_EACH_LIR_LITERAL(V)
#undef V

// Print for formatting and debugging.
ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Literal& literal);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const BasicBlock& basic_block);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Function& function);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_LITERALS_FORWARD_H_
