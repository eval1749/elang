// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_TYPE_VALUES_H_
#define ELANG_COMPILER_ANALYSIS_TYPE_VALUES_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/analysis/type_values_forward.h"

namespace elang {
namespace compiler {

namespace ast {
class Call;
class Node;
}

namespace ir {
class Method;
class Type;
}

class TypeResolver;

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
  friend class Evaluator;                              \
  friend class Factory;                                \
  friend class TypeResolver;

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

//////////////////////////////////////////////////////////////////////
//
// UnionValue
//
class UnionValue : public Value {
  DECLARE_ABSTRACT_TYPE_VALUE_CLASS(UnionValue, Value);

 public:
  virtual const ZoneVector<ir::Method*>& methods() const = 0;
  virtual ir::Type* valueFor(const ir::Method* method) const = 0;

  virtual bool CanUse(ir::Method* method, ir::Type* type) const = 0;
  virtual void SetMethods(const std::vector<ir::Method*>& methods) = 0;

 protected:
  UnionValue();

 private:
  DISALLOW_COPY_AND_ASSIGN(UnionValue);
};

//////////////////////////////////////////////////////////////////////
//
// AnyValue
//
class AnyValue final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(AnyValue, Value);

 private:
  AnyValue();

  DISALLOW_COPY_AND_ASSIGN(AnyValue);
};

//////////////////////////////////////////////////////////////////////
//
// AndValue
//
class AndValue final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(AndValue, Value);

 public:
  const ZoneVector<UnionValue*>& union_values() const { return union_values_; }

 private:
  explicit AndValue(Zone* zone, const std::vector<UnionValue*>& union_values);

  void SetUnionValues(const std::vector<UnionValue*>& union_values);

  ZoneVector<UnionValue*> union_values_;

  DISALLOW_COPY_AND_ASSIGN(AndValue);
};

//////////////////////////////////////////////////////////////////////
//
// Argument
//
class Argument final : public UnionValue {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(Argument, UnionValue);

 public:
  CallValue* call_value() const { return call_value_; }
  const ZoneVector<ir::Method*>& methods() const final;
  int position() const { return position_; }
  ir::Type* valueFor(const ir::Method* method) const final;

  void SetMethods(const std::vector<ir::Method*>& methods);

  // UnionValue
  bool CanUse(ir::Method* method, ir::Type* type) const final;

 private:
  Argument(CallValue* call_value, int position);

  CallValue* const call_value_;
  int const position_;

  DISALLOW_COPY_AND_ASSIGN(Argument);
};

//////////////////////////////////////////////////////////////////////
//
// CallValue
//
class CallValue final : public UnionValue {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(CallValue, UnionValue);

 public:
  ast::Call* ast_call() const { return ast_call_; }
  const ZoneVector<ir::Method*>& methods() const final { return methods_; }
  ir::Type* valueFor(const ir::Method* method) const final;

  void SetMethods(const std::vector<ir::Method*>& methods) final;

  // UnionValue
  bool CanUse(ir::Method* method, ir::Type* type) const final;

 private:
  CallValue(Zone* zone, ast::Call* ast_call);

  ast::Call* const ast_call_;
  ZoneVector<ir::Method*> methods_;

  DISALLOW_COPY_AND_ASSIGN(CallValue);
};

//////////////////////////////////////////////////////////////////////
//
// EmptyValue
//
class EmptyValue final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(EmptyValue, Value);

 private:
  EmptyValue();

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

  ir::Type* const value_;

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

// Represents 'null' literal of type |value|.
class NullValue final : public Value {
  DECLARE_CONCRETE_TYPE_VALUE_CLASS(NullValue, Value);

 public:
  Value* value() const { return value_; }

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

  Variable* Find() const;

 private:
  explicit Variable(ast::Node* node, Value* value);

  ast::Node* const node_;
  mutable Variable* parent_;
  int rank_;
  Value* value_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_TYPE_VALUES_H_
