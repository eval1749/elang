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
  V(Control)                                \
  V(Effect)                                 \
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
  V(DynamicCast, "dynamic_cast", Node)               \
  V(Exit, "exit", Node)                              \
  V(IfException, "if_exception", Control)            \
  V(IfFalse, "if_false", Control)                    \
  V(IfSuccess, "if_success", Control)                \
  V(IfTrue, "if_true", Control)                      \
  V(Jump, "br", Control)                             \
  V(StaticCast, "static_cast", Node)                 \
  V(Unreachable, "unreachable", Control)

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_2(V) \
  V(If, "if", Control)                               \
  V(PhiOperand, "phi_operand", Node)                 \
  V(IntShl, "shl", Node)                             \
  V(IntShr, "shr", Node)                             \
  V(StackAlloc, "alloca", Node)                      \
  V(Switch, "switch", Control)                       \
  V(Throw, "throw", Control)

// TODO(eval1749) How do we represent integer division-by-zero?
#define FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V) \
  V(FloatAdd, "fadd", Node)                            \
  V(FloatDiv, "fdiv", Node)                            \
  V(FloatMod, "fmod", Node)                            \
  V(FloatMul, "fmul", Node)                            \
  V(FloatSub, "fsub", Node)                            \
  V(IntBitAnd, "bit_and", Node)                        \
  V(IntBitOr, "bit_or", Node)                          \
  V(IntBitXor, "bit_xor", Node)                        \
  V(IntAdd, "add", Node)                               \
  V(IntDiv, "div", Node)                               \
  V(IntMod, "mod", Node)                               \
  V(IntMul, "mul", Node)                               \
  V(IntSub, "sub", Node)

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_3(V) \
  V(Call, "call", Node)                              \
  V(Load, "load", Node)                              \
  V(Ret, "ret", Control)

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_V(V) \
  V(Case, "case", Control)                           \
  V(Tuple, "tuple", Node)

#define FOR_EACH_OPTIMIZER_CONCRETE_NODE_X(V) \
  V(ControlGet, "control_get", Control)       \
  V(EffectGet, "effect_get", Effect)          \
  V(EffectPhi, "effect_phi", Node)            \
  V(Entry, "entry", Node)                     \
  V(FloatCmp, "fcmp", Node)                   \
  V(FunctionReference, "fn", Node)            \
  V(Get, "get", Node)                         \
  V(IntCmp, "cmp", Node)                      \
  V(Loop, "loop", Control)                    \
  V(Merge, "merge", Control)                  \
  V(Null, "lit_null", Node)                   \
  V(Parameter, "param", Node)                 \
  V(Phi, "phi", Node)                         \
  V(Reference, "ref", Node)                   \
  V(SizeOf, "sizeof", Node)                   \
  V(Store, "store", Node)                     \
  V(Void, "void", Node)

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
