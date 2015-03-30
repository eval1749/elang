// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_NODES_FORWARD_H_
#define ELANG_OPTIMIZER_NODES_FORWARD_H_

#include <ostream>

#include "base/basictypes.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

// List of abstract C++ classes representing IR node.
#define FOR_EACH_OPTIMIZER_ABSTRACT_NODE(V) \
  V(FieldInputNode)                         \
  V(Node)                                   \
  V(PhiOwnerNode)

#define FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V) \
  V(Bool, "lit_bool", bool)                         \
  V(Char, "lit_char", base::char16)                 \
  V(Float32, "lit_f32", float32_t)                  \
  V(Float64, "lit_f64", float64_t)                  \
  V(Int16, "lit_i16", int16_t)                      \
  V(Int32, "lit_i32", int32_t)                      \
  V(Int64, "lit_i64", int64_t)                      \
  V(Int8, "lit_i8", int8_t)                         \
  V(IntPtr, "lit_iptr", intptr_t)                   \
  V(String, "lit_string", base::StringPiece16)      \
  V(UInt16, "lit_u16", uint16_t)                    \
  V(UInt32, "lit_u32", uint32_t)                    \
  V(UInt64, "lit_u64", uint64_t)                    \
  V(UInt8, "lit_u8", uint8_t)                       \
  V(UIntPtr, "lit_uptr", uintptr_t)

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_1(V) \
  V(DynamicCast, "dynamic_cast")                     \
  V(Exit, "exit")                                    \
  V(IfException, "if_exception")                     \
  V(IfFalse, "if_false")                             \
  V(IfSuccess, "if_success")                         \
  V(IfTrue, "if_true")                               \
  V(Jump, "br")                                      \
  V(StaticCast, "static_cast")                       \
  V(Switch, "switch")                                \
  V(Unreachable, "unreachable")

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_2(V) \
  V(If, "if")                                        \
  V(PhiOperand, "phi_operand")                       \
  V(IntShl, "shl")                                   \
  V(IntShr, "shr")                                   \
  V(StackAlloc, "alloca")                            \
  V(Throw, "throw")

// TODO(eval1749) How do we represent integer division-by-zero?
#define FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V) \
  V(FloatAdd, "fadd")                                  \
  V(FloatDiv, "fdiv")                                  \
  V(FloatMod, "fmod")                                  \
  V(FloatMul, "fmul")                                  \
  V(FloatSub, "fsub")                                  \
  V(IntBitAnd, "bit_and")                              \
  V(IntBitOr, "bit_or")                                \
  V(IntBitXor, "bit_xor")                              \
  V(IntAdd, "add")                                     \
  V(IntDiv, "div")                                     \
  V(IntMod, "mod")                                     \
  V(IntMul, "mul")                                     \
  V(IntSub, "sub")

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_3(V) \
  V(Call, "call")                                    \
  V(Load, "load")                                    \
  V(Ret, "ret")

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_V(V) \
  V(Case, "case")                                    \
  V(Tuple, "tuple")

#define FOR_EACH_OPTIMIZER_CONCRETE_NODE_X(V) \
  V(EffectGet, "effect_get")                  \
  V(EffectPhi, "effect_phi")                  \
  V(Entry, "entry")                           \
  V(FloatCmp, "fcmp")                         \
  V(FunctionReference, "fn")                  \
  V(Get, "get")                               \
  V(IntCmp, "cmp")                            \
  V(Loop, "loop")                             \
  V(Merge, "merge")                           \
  V(Null, "lit_null")                         \
  V(Parameter, "param")                       \
  V(Phi, "phi")                               \
  V(Reference, "ref")                         \
  V(SizeOf, "sizeof")                         \
  V(Store, "store")                           \
  V(Void, "void")

// List of concrete C++ classes representing IR node.
#define FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)      \
  FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V) \
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)    \
  FOR_EACH_OPTIMIZER_CONCRETE_NODE_X(V)          \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_1(V)   \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_2(V)   \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_3(V)   \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_V(V)

// Forward declarations
#define V(Name) class Name;
FOR_EACH_OPTIMIZER_ABSTRACT_NODE(V)
#undef V

#define V(Name, ...) class Name##Node;
FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V

using Effect = Node;
class Factory;
enum class FloatCondition;
class Function;
enum class IntCondition;
class NodeFactory;
typedef uint32_t NodeId;
class NodeVisitor;
enum class Opcode;

// Print for formatting and debugging.
ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                FloatCondition condition);

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                Function function);

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                IntCondition condition);

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const Node& node);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODES_FORWARD_H_
