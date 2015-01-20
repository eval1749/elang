// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_VALUES_H_
#define ELANG_COMPILER_ANALYZE_TYPE_VALUES_H_

#include <ostream>
#include <vector>

#include "base/macros.h"
#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/analyze/type_values_forward.h"

namespace elang {
namespace compiler {
namespace ast {
class Node;
}

namespace ir {
class Type;
}

namespace ts {

#define DECLARE_TYPE_VALUE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);        \
                                              \
 protected:                                   \
  ~self() = default;

#define DECLARE_ABSTRACT_TYPE_VALUE_CLASS(self, super) \
  DECLARE_TYPE_VALUE_CLASS(self, super);

#define DECLARE_CONCRETE_TYPE_VALUE_CLASS(self, super) \
  DECLARE_TYPE_VALUE_CLASS(self, super);               \
                                                       \
 private:                                              \
  /* |Factory| class if friend of concrete |Node| */   \
  /* class, for accessing constructor. */              \
  friend class Factory;

//////////////////////////////////////////////////////////////////////
//
// Value
//
class Value : public Castable, public ZoneAllocated {
  DECLARE_ABSTRACT_TYPE_VALUE_CLASS(Value, Castable);

 public:
  virtual bool Contains(const Value* other) const = 0;

 protected:
  Value();

 private:
  DISALLOW_COPY_AND_ASSIGN(Value);
};

std::ostream& operator<<(std::ostream& ostream, const Value& value);

//////////////////////////////////////////////////////////////////////
//
// AnyValue
//
class AnyValue final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(AnyValue, Value);

 private:
  AnyValue();

  // Value
  bool Contains(const Value* other) const final;

  DISALLOW_COPY_AND_ASSIGN(AnyValue);
};

//////////////////////////////////////////////////////////////////////
//
// EmptyValue
//
class EmptyValue final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(EmptyValue, Value);

 private:
  EmptyValue();

  // Value
  bool Contains(const Value* other) const final;

  DISALLOW_COPY_AND_ASSIGN(EmptyValue);
};

//////////////////////////////////////////////////////////////////////
//
// InvalidValue
//
class InvalidValue final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(InvalidValue, Value);

 public:
  ast::Node* node() const { return node_; }

 private:
  explicit InvalidValue(ast::Node* node);

  // Value
  bool Contains(const Value* other) const final;

  ast::Node* const node_;

  DISALLOW_COPY_AND_ASSIGN(InvalidValue);
};

// Represents literal type
class Literal final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(Literal, Value);

 public:
  ir::Type* value() const { return value_; }

 private:
  explicit Literal(ir::Type* value);

  // Value
  bool Contains(const Value* other) const final;

  ir::Type* const value_;

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

// Represents 'null' literal of type |value|.
class NullValue final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(NullValue, Value);

 public:
  Value* value() const { return value_; }

  // Value
  bool Contains(const Value* other) const final;

 private:
  explicit NullValue(Value* value);

  Value* const value_;

  DISALLOW_COPY_AND_ASSIGN(NullValue);
};

// Type variable for |node|.
class Variable final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(Variable, Value);

 public:
  ast::Node* node() const { return node_; }
  Value* value() const { return value_; }

  // Value
  bool Contains(const Value* other) const final;

 private:
  explicit Variable(ast::Node* node, Value* value);

  ast::Node* const node_;
  Value* value_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_VALUES_H_
