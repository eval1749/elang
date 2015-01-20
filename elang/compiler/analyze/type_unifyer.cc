// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/type_unifyer.h"

#include "base/logging.h"
#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/ir/nodes.h"

namespace elang {
namespace compiler {
namespace ts {

//////////////////////////////////////////////////////////////////////
//
// TypeUnifyer
//
TypeUnifyer::TypeUnifyer(Factory* factory) : factory_(factory) {
}

TypeUnifyer::~TypeUnifyer() {
}

bool TypeUnifyer::Contains(const AndValue* and_value1, const ir::Type* type2) {
  for (auto const union_value1 : and_value1->union_values()) {
    if (Contains(union_value1, type2))
      return true;
  }
  return false;
}

bool TypeUnifyer::Contains(const UnionValue* union_value1,
                           const ir::Type* type2) {
  for (auto const method1 : union_value1->methods()) {
    if (union_value1->value(method1)->IsSubtypeOf(type2))
      return true;
  }
  return false;
}

Value* TypeUnifyer::GetAnyValue() {
  return factory()->GetAnyValue();
}

Value* TypeUnifyer::GetEmptyValue() {
  return factory()->GetEmptyValue();
}

Value* TypeUnifyer::NewLiteral(ir::Type* type) {
  return factory()->NewLiteral(type);
}

// The entry point of |TypeUnifyer|.
Value* TypeUnifyer::Unify(Value* value1, Value* value2) {
  if (value1->is<InvalidValue>() || value1->is<EmptyValue>())
    return value1;
  if (value2->is<InvalidValue>() || value2->is<EmptyValue>())
    return value2;

  if (value1->is<AnyValue>())
    return value2;
  if (value2->is<AnyValue>())
    return value1;

  if (auto const variable1 = value1->as<Variable>())
    return Unify(variable1, value2);

  if (auto const variable2 = value2->as<Variable>())
    return Unify(variable2, value1);

  if (auto const null1 = value1->as<NullValue>())
    return Unify(null1->value(), value2);

  if (auto const null2 = value2->as<NullValue>())
    return Unify(null2->value(), value1);

  if (auto const literal1 = value1->as<Literal>())
    return Unify(literal1, value2);

  if (auto const literal2 = value2->as<Literal>())
    return Unify(literal2, value1);

  if (auto const union_value1 = value1->as<UnionValue>())
    return Unify(union_value1, value2);

  if (auto const union_value2 = value2->as<UnionValue>())
    return Unify(union_value2, value1);

  if (auto const and_value1 = value1->as<AndValue>())
    return Unify(and_value1, value2);

  NOTREACHED() << "Unify(" << *value1 << ", " << value2 << ")";
  return GetEmptyValue();
}

// Unify AndValue
Value* TypeUnifyer::Unify(AndValue* and_value1, AndValue* and_value2) {
  NOTREACHED() << "Unify(" << *and_value1 << ", " << and_value2 << ")";
  return GetEmptyValue();
}

Value* TypeUnifyer::Unify(AndValue* and_value1, Value* value2) {
  if (auto const and_value2 = value2->as<AndValue>())
    return Unify(and_value1, and_value2);

  NOTREACHED() << "Unify(" << *and_value1 << ", " << value2 << ")";
  return GetEmptyValue();
}

// Unify Literal
Value* TypeUnifyer::Unify(Literal* literal1, AndValue* and_value2) {
  std::vector<UnionValue*> union_values;
  auto type1 = static_cast<Value*>(literal1);
  for (auto const union_value2 : and_value2->union_values()) {
    auto const value = Unify(type1, union_value2);
    if (auto const union_value = value->as<UnionValue>()) {
      union_values.push_back(union_value);
      continue;
    }
    // Note: even if |type1| is empty value, we call |Unify()| for all
    // |UnionValue| in |and_value2| to update method list in |UnionValue|.
    type1 = Unify(type1, value);
  }
  if (union_values.empty())
    return type1;
  if (union_values.size() == 1u)
    return union_values.front();
  and_value2->SetUnionValues(union_values);
  return and_value2;
}

Value* TypeUnifyer::Unify(Literal* literal1, UnionValue* union_value2) {
  auto const type1 = literal1->value();
  std::vector<ir::Method*> methods;
  for (auto const method : union_value2->methods()) {
    if (method->return_type()->IsSubtypeOf(type1))
      methods.push_back(method);
  }
  union_value2->SetMethods(methods);
  if (methods.empty())
    return GetEmptyValue();
  if (methods.size() == 1)
    return NewLiteral(methods.front()->return_type());
  return union_value2;
}

Value* TypeUnifyer::Unify(Literal* literal1, Literal* literal2) {
  if (literal1->value()->IsSubtypeOf(literal2->value()))
    return literal1;
  if (literal2->value()->IsSubtypeOf(literal1->value()))
    return literal2;
  return GetEmptyValue();
}

Value* TypeUnifyer::Unify(Literal* literal1, Value* value2) {
  if (auto const and2 = value2->as<AndValue>())
    return Unify(literal1, and2);

  if (auto const literal2 = value2->as<Literal>())
    return Unify(literal1, literal2);

  if (auto const union2 = value2->as<UnionValue>())
    return Unify(literal1, union2);

  NOTREACHED() << "unify(" << *literal1 << ", " << value2 << ")";
  return GetEmptyValue();
}

// Unify UnionValue
Value* TypeUnifyer::Unify(UnionValue* union_value1, AndValue* and_value2) {
  std::vector<ir::Method*> methods1;
  for (auto const method1 : union_value1->methods()) {
    if (Contains(and_value2, union_value1->value(method1)))
      methods1.push_back(method1);
  }
  union_value1->SetMethods(methods1);
  if (methods1.empty())
    return GetEmptyValue();
  if (methods1.size() == 1u)
    return NewLiteral(methods1.front()->return_type());
  auto const and_value = factory()->NewAndValue();
  std::vector<UnionValue*> union_values(and_value2->union_values().begin(),
                                        and_value2->union_values().end());
  union_values.push_back(union_value1);
  and_value->SetUnionValues(union_values);
  return and_value;
}

Value* TypeUnifyer::Unify(UnionValue* union_value1, UnionValue* union_value2) {
  std::vector<ir::Method*> methods1;
  for (auto const method1 : union_value1->methods()) {
    if (Contains(union_value2, union_value1->value(method1)))
      methods1.push_back(method1);
  }
  union_value1->SetMethods(methods1);
  if (methods1.empty()) {
    union_value2->SetMethods({});
    return GetEmptyValue();
  }
  if (methods1.size() == 1u)
    return Unify(NewLiteral(methods1.front()->return_type()), union_value2);

  std::vector<ir::Method*> methods2;
  for (auto const method2 : union_value2->methods()) {
    if (Contains(union_value1, method2->return_type()))
      methods2.push_back(method2);
  }
  union_value2->SetMethods(methods2);
  if (methods2.empty())
    return GetEmptyValue();
  if (methods2.size() == 1u)
    return NewLiteral(methods2.front()->return_type());
  return union_value2;
}

Value* TypeUnifyer::Unify(UnionValue* union_value1, Value* value2) {
  if (auto const and_value2 = value2->as<AndValue>())
    return Unify(union_value1, and_value2);

  if (auto const union_value2 = value2->as<UnionValue>())
    return Unify(union_value1, union_value2);

  NOTREACHED() << "Unify(" << *union_value1 << ", " << value2 << ")";
  return GetEmptyValue();
}

// Unify Variable
Value* TypeUnifyer::Unify(Variable* variable1, Value* value2) {
  if (auto const variable2 = value2->as<Variable>())
    return Unify(variable1, variable2);
  auto const result = Unify(variable1->value(), value2);
  variable1->value_ = result;
  return result;
}

Value* TypeUnifyer::Unify(Variable* variable1, Variable* variable2) {
  auto const result = Unify(variable1->value(), variable2->value());
  variable1->value_ = result;
  variable2->value_ = result;
  Union(variable1, variable2);
  return result;
}

void TypeUnifyer::Union(Variable* variable1, Variable* variable2) {
  auto root1 = variable1;
  while (root1 != variable1)
    root1 = root1->parent_;
  auto root2 = variable1;
  while (root2 != variable2)
    root2 = root2->parent_;
  if (root1 == root2)
    return;
  if (root1->rank_ < root2->rank_) {
    root1->parent_ = root2;
    return;
  }
  if (root1->rank_ > root2->rank_) {
    root2->parent_ = root1;
    return;
  }
  root2->parent_ = root1;
  ++root1->rank_;
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
