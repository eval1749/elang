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
class AnyValue : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(AnyValue, Value);

 private:
  AnyValue();

  DISALLOW_COPY_AND_ASSIGN(AnyValue);
};

//////////////////////////////////////////////////////////////////////
//
// EmptyValue
//
class EmptyValue : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(EmptyValue, Value);

 private:
  EmptyValue();

  DISALLOW_COPY_AND_ASSIGN(EmptyValue);
};

//////////////////////////////////////////////////////////////////////
//
// InvalidValue
//
class InvalidValue : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(InvalidValue, Value);

  ast::Node* node() const { return node_; }

 private:
  explicit InvalidValue(ast::Node* node);

  ast::Node* const node_;

  DISALLOW_COPY_AND_ASSIGN(InvalidValue);
};

// Represents literal type
class Literal : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(Literal, Value);

  ir::Type* value() const { return value_; }

 private:
  explicit Literal(ir::Type* value);

  ir::Type* const value_;

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

// Represents 'null' literal of type |value|.
class NullValue : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(NullValue, Value);

  Value* value() const { return value_; }

 private:
  explicit NullValue(Value* value);

  Value* const value_;

  DISALLOW_COPY_AND_ASSIGN(NullValue);
};

// Type variable for |node|.
class Variable : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(Variable, Value);

  ast::Node* node() const { return node_; }

 private:
  explicit Variable(ast::Node* node);

  ast::Node* const node_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_VALUES_H_
