// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_VALUE_H_
#define ELANG_LIR_VALUE_H_

#include <stdint.h>
#include <iosfwd>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "elang/base/float_types.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

// If you change definition of |ValueSize|, please change
// |ShiftCountOf(ValueSize)| and |BitSizeOf(size)| appropriately.
enum class ValueSize : uint32_t {
  Size8,
  Size16,
  Size32,
  Size64,
  NotUsed4,
  NotUsed5,
  NotUsed6,
  Size0,
};

static_assert(static_cast<size_t>(ValueSize::Size0) == 7,
              "ValueSize::Size0 must be 7");

#define FOR_EACH_VALUE_KIND(V)                            \
  V(Void)                                                 \
  V(Immediate)                                            \
  V(Literal)                                              \
  V(Parameter)                                            \
  V(PhysicalRegister)                                     \
  V(VirtualRegister)                                      \
  V(Conditional) /* output of cmp/fcmp instructions. */   \
  V(Argument)                                             \
  V(StackSlot) /* stack location for spilled registers */ \
  V(SpillSlot)                                            \
  V(FrameSlot)                                            \
  V(NotUsed11)                                            \
  V(NotUsed12)                                            \
  V(NotUsed13)                                            \
  V(NotUsed14)                                            \
  V(Instruction) /* for ErrorData */

//////////////////////////////////////////////////////////////////////
//
// Value represents both input and output operand of instruction.
//
struct ELANG_LIR_EXPORT Value {
  enum class Type : uint32_t {
    Integer,
    Float,
  };

  enum class Kind : uint32_t {
#define V(Name) Name,
    FOR_EACH_VALUE_KIND(V)
#undef V
  };

  static const int kMaximumImmediate = 1 << 23;
  static const int kMinimumImmediate = -1 << 23;

  Type type : 1;
  ValueSize size : 3;
  Kind kind : 4;
  int data : 24;

  Value(Type type, ValueSize size, Kind kind, int data);
  Value();

  // predicates for |ValueSize|
  bool is_8bit() const { return size == ValueSize::Size8; }
  bool is_16bit() const { return size == ValueSize::Size16; }
  bool is_32bit() const { return size == ValueSize::Size32; }
  bool is_64bit() const { return size == ValueSize::Size64; }

  // predicates for |Type|
  bool is_float() const { return type == Type::Float; }
  bool is_float32() const { return is_float() && is_32bit(); }
  bool is_float64() const { return is_float() && is_64bit(); }
  bool is_int8() const { return is_integer() && is_8bit(); }
  bool is_int16() const { return is_integer() && is_16bit(); }
  bool is_int32() const { return is_integer() && is_32bit(); }
  bool is_int64() const { return is_integer() && is_64bit(); }
  bool is_integer() const { return type == Type::Integer; }

  // predicates for |Kind|
  bool is_argument() const { return kind == Kind::Argument; }
  bool is_conditional() const { return kind == Kind::Conditional; }
  bool is_frame_slot() const { return kind == Kind::FrameSlot; }
  bool is_immediate() const { return kind == Kind::Immediate; }
  bool is_instruction() const { return kind == Kind::Instruction; }
  bool is_literal() const { return kind == Kind::Literal; }
  bool is_memory_proxy() const;
  bool is_memory_slot() const;
  bool is_output() const;
  bool is_register() const { return is_physical() || is_virtual(); }
  bool is_parameter() const { return kind == Kind::Parameter; }
  bool is_physical() const { return kind == Kind::PhysicalRegister; }
  bool is_read_only() const { return is_immediate() || is_literal(); }
  bool is_spill_slot() const { return kind == Kind::SpillSlot; }
  bool is_stack_slot() const { return kind == Kind::StackSlot; }
  bool is_virtual() const { return kind == Kind::VirtualRegister; }
  bool is_void() const { return kind == Kind::Void; }
  bool is_void_type() const;

  // Helper functions
  static bool CanBeImmediate(int64_t value);

  // |ValueSize| properties
  static int BitSizeOf(Value value) { return SizeOf(value) * 8; }
  static int SizeOf(Value value) { return 1 << (Log2Of(value) - 3); }
  static int Log2Of(Value value) { return static_cast<int>(value.size) + 3; }

  // |Value::Type|
  static Value TypeOf(Value value);

  static Value Argument(Value type, int data);
  static Value False();
  static Value Float32Literal();
  static Value Float64Literal();
  static Value Float32Type();
  static Value Float64Type();
  static Value FrameSlot(Value type, int data);
  static Value Int16Type();
  static Value Int32Type();
  static Value Int64Type();
  static Value Int8Type();
  static Value IntPtrType();
  static Value Literal(Value type);
  static Value Parameter(Value type, int data);
  static Value Register(Value type, int data);
  static Value SpillSlot(Value type, int data);
  static Value StackSlot(Value type, int data);
  static Value SmallInt16(int data);
  static Value SmallInt32(int data);
  static Value SmallInt64(int data);
  static Value SmallInt8(int data);
  static Value True();
  static Value Void();
  static Value VoidType();

 private:
  friend class Factory;

  Value(Value type, Kind kind) : Value(type.type, type.size, kind, 0) {}
  Value(Type type, ValueSize size) : Value(type, size, Kind::Void, 0) {}
  static Value Immediate(ValueSize size, int data);
};

static_assert(sizeof(Value) == sizeof(int),
              "Value must be packed into 32bit integer.");

inline bool operator==(const Value& value1, const Value& value2) {
  return value1.data == value2.data && value1.kind == value2.kind &&
         value1.size == value2.size && value1.type == value2.type;
}

inline bool operator!=(const Value& value1, const Value& value2) {
  return !operator==(value1, value2);
}

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value& value);

ELANG_LIR_EXPORT std::ostream& operator<<(
    std::ostream& ostream,
    const std::pair<Value, Value>& values);

ELANG_LIR_EXPORT std::ostream& operator<<(
    std::ostream& ostream,
    const std::unordered_map<Value, Value>& pairs);

ELANG_LIR_EXPORT std::ostream& operator<<(
    std::ostream& ostream,
    const std::unordered_set<Value>& values);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const std::vector<Value>& values);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value::Kind& kind);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const ValueSize& size);

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value::Type& type);

}  // namespace lir
}  // namespace elang

namespace std {
template <>
struct ELANG_LIR_EXPORT hash<elang::lir::Value> {
  size_t operator()(const elang::lir::Value& pair) const;
};
}  // namespace std

#endif  // ELANG_LIR_VALUE_H_
