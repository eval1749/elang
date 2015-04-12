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
  V(Data)                                   \
  V(Effect)                                 \
  V(Literal)                                \
  V(Node)                                   \
  V(PhiOwnerNode)                           \
  V(Tuple)

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
  V(DynamicCast, "dynamic_cast", Data)               \
  V(Exit, "exit", Control)                           \
  V(GetData, "get_data", Data)                       \
  V(GetEffect, "get_effect", Effect)                 \
  V(GetTuple, "get_tuple", Tuple)                    \
  V(IfException, "if_exception", Control)            \
  V(IfFalse, "if_false", Control)                    \
  V(IfSuccess, "if_success", Control)                \
  V(IfTrue, "if_true", Control)                      \
  V(Jump, "br", Control)                             \
  V(StaticCast, "static_cast", Data)                 \
  V(Unreachable, "unreachable", Control)

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_2(V) \
  V(Element, "element", Data)                        \
  V(If, "if", Control)                               \
  V(IntShl, "shl", Data)                             \
  V(IntShr, "shr", Data)                             \
  V(Length, "length", Data)                          \
  V(StackAlloc, "alloca", Data)                      \
  V(Switch, "switch", Control)                       \
  V(Throw, "throw", Control)

// TODO(eval1749) How do we represent integer division-by-zero?
#define FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V) \
  V(FloatAdd, "fadd", Data)                            \
  V(FloatDiv, "fdiv", Data)                            \
  V(FloatMod, "fmod", Data)                            \
  V(FloatMul, "fmul", Data)                            \
  V(FloatSub, "fsub", Data)                            \
  V(IntBitAnd, "bit_and", Data)                        \
  V(IntBitOr, "bit_or", Data)                          \
  V(IntBitXor, "bit_xor", Data)                        \
  V(IntAdd, "add", Data)                               \
  V(IntDiv, "div", Data)                               \
  V(IntMod, "mod", Data)                               \
  V(IntMul, "mul", Data)                               \
  V(IntSub, "sub", Data)

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_3(V) \
  V(Load, "load", Data)                              \
  V(Ret, "ret", Control)

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_4(V) V(Call, "call", Control)

#define FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_V(V) \
  V(Case, "case", Control)                           \
  V(Tuple, "tuple", Tuple)

#define FOR_EACH_OPTIMIZER_CONCRETE_NODE_X(V) \
  V(EffectPhi, "effect_phi", Effect)          \
  V(Entry, "entry", Control)                  \
  V(FloatCmp, "fcmp", Data)                   \
  V(FunctionReference, "fn", Data)            \
  V(Get, "get", Data)                         \
  V(IntCmp, "cmp", Data)                      \
  V(Loop, "loop", Control)                    \
  V(Merge, "merge", Control)                  \
  V(Null, "lit_null", Literal)                \
  V(Parameter, "param", Data)                 \
  V(Phi, "phi", Data)                         \
  V(Reference, "ref", Literal)                \
  V(SizeOf, "sizeof", Literal)                \
  V(Store, "store", Literal)                  \
  V(Void, "void", Literal)

// List of concrete C++ classes representing IR node.
#define FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)      \
  FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V) \
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)    \
  FOR_EACH_OPTIMIZER_CONCRETE_NODE_X(V)          \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_1(V)   \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_2(V)   \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_3(V)   \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_4(V)   \
  FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_V(V)

#define FOR_EACH_OPTIMIZER_BLOCK_START_NODE(V) \
  V(Entry)                                     \
  V(Loop)                                      \
  V(Merge)

#define FOR_EACH_OPTIMIZER_EDGE_NODE(V) \
  V(Case)                               \
  V(IfException)                        \
  V(IfFalse)                            \
  V(IfSuccess)                          \
  V(IfTrue)

#define FOR_EACH_OPTIMIZER_BLOCK_END_NODE(V) \
  V(Exit)                                    \
  V(If)                                      \
  V(Ret)                                     \
  V(Switch)                                  \
  V(Throw)

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
