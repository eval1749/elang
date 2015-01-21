// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>
#include <vector>

#include "elang/compiler/analyze/type_evaluator.h"

#include "base/logging.h"
#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/ir/nodes.h"

namespace elang {
namespace compiler {
namespace ts {

//////////////////////////////////////////////////////////////////////
//
// Evaluator
//
Evaluator::Evaluator(Factory* factory) : factory_(factory) {
}

Evaluator::~Evaluator() {
}

bool Evaluator::Contains(const AndValue* and_value1, ir::Type* type2) {
  for (auto const union_value1 : and_value1->union_values()) {
    if (Contains(union_value1, type2))
      return true;
  }
  return false;
}

bool Evaluator::Contains(const AndValue* and_value1,
                           const UnionValue* union_value2) {
  for (auto const method2 : union_value2->methods()) {
    if (Contains(and_value1, union_value2->value(method2)))
      return true;
  }
  return false;
}

// Returns true if we can use |type2| with |union_value1|.
bool Evaluator::Contains(const UnionValue* union_value1, ir::Type* type2) {
  for (auto const method1 : union_value1->methods()) {
    if (union_value1->CanUse(method1, type2))
      return true;
  }
  return false;
}

Value* Evaluator::Evaluate(ts::Value* value) {
  if (auto const and_value = value->as<AndValue>()) {
    ts::Value* result = nullptr;
    for (auto const union_value : and_value->union_values()) {
      if (!result) {
        result = Evaluate(union_value);
        continue;
      }
      if (result != Evaluate(union_value))
        return value;
    }
    return result ? result : GetEmptyValue();
  }
  if (auto const union_value = value->as<UnionValue>()) {
    ir::Type* result = nullptr;
    for (auto const method : union_value->methods()) {
      if (!result) {
        result = union_value->value(method);
        continue;
      }
      if (result != union_value->value(method))
        return value;
    }
    return result ? NewLiteral(result) : GetEmptyValue();
  }

  if (auto const variable = value->as<Variable>()) {
    auto const variable_value = variable->Find()->value();
    DCHECK(!variable_value->is<Variable>());
    return Evaluate(variable_value);
  }
  return value;
}

Value* Evaluator::GetAnyValue() {
  return factory()->GetAnyValue();
}

Value* Evaluator::GetEmptyValue() {
  return factory()->GetEmptyValue();
}

Value* Evaluator::NewLiteral(ir::Type* type) {
  return factory()->NewLiteral(type);
}

// The entry point of |Evaluator|.
Value* Evaluator::Unify(Value* value1, Value* value2) {
  if (value1 == value2)
    return value1;

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
Value* Evaluator::Unify(AndValue* and_value1, AndValue* and_value2) {
  std::unordered_set<UnionValue*> union_values;
  std::vector<UnionValue*> union_values1;
  for (auto const union_value1 : and_value1->union_values()) {
    if (!Contains(and_value2, union_value1))
      continue;
    union_values1.push_back(union_value1);
    union_values.insert(union_value1);
  }
  and_value1->SetUnionValues(union_values1);
  if (union_values1.empty()) {
    and_value2->SetUnionValues({});
    return GetEmptyValue();
  }
  if (union_values1.size() == 1u)
    return Unify(union_values1.front(), and_value2);

  std::vector<UnionValue*> union_values2;
  for (auto const union_value2 : and_value2->union_values()) {
    if (!Contains(and_value1, union_value2))
      continue;
    union_values2.push_back(union_value2);
    union_values.insert(union_value2);
  }
  and_value2->SetUnionValues(union_values2);
  DCHECK(!union_values2.empty());
  DCHECK(!union_values.empty());
  if (union_values2.size() == 1u)
    return Unify(union_values2.front(), and_value1);

  // We still need to have as |AndValue(and_value1 + and_value2)|.
  std::vector<UnionValue*> result(union_values.begin(), union_values.end());
  return factory()->NewAndValue(result);
}

Value* Evaluator::Unify(AndValue* and_value1, Value* value2) {
  if (auto const and_value2 = value2->as<AndValue>())
    return Unify(and_value1, and_value2);

  NOTREACHED() << "Unify(" << *and_value1 << ", " << value2 << ")";
  return GetEmptyValue();
}

// Unify Literal
Value* Evaluator::Unify(Literal* literal1, AndValue* and_value2) {
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

Value* Evaluator::Unify(Literal* literal1, UnionValue* union_value2) {
  auto const type1 = literal1->value();
  std::vector<ir::Method*> methods2;
  for (auto const method2 : union_value2->methods()) {
    if (union_value2->CanUse(method2, type1))
      methods2.push_back(method2);
  }
  union_value2->SetMethods(methods2);
  if (methods2.empty())
    return GetEmptyValue();
  if (methods2.size() == 1)
    return NewLiteral(union_value2->value(methods2.front()));
  return union_value2;
}

Value* Evaluator::Unify(Literal* literal1, Literal* literal2) {
  if (literal1->value()->IsSubtypeOf(literal2->value()))
    return literal1;
  if (literal2->value()->IsSubtypeOf(literal1->value()))
    return literal2;
  return GetEmptyValue();
}

Value* Evaluator::Unify(Literal* literal1, Value* value2) {
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
Value* Evaluator::Unify(UnionValue* union_value1, AndValue* and_value2) {
  std::vector<ir::Method*> methods1;
  for (auto const method1 : union_value1->methods()) {
    if (Contains(and_value2, union_value1->value(method1)))
      methods1.push_back(method1);
  }
  union_value1->SetMethods(methods1);
  if (methods1.empty())
    return GetEmptyValue();
  if (methods1.size() == 1u)
    return NewLiteral(union_value1->value(methods1.front()));
  std::vector<UnionValue*> union_values(and_value2->union_values().begin(),
                                        and_value2->union_values().end());
  union_values.push_back(union_value1);
  return factory()->NewAndValue(union_values);
}

Value* Evaluator::Unify(UnionValue* union_value1, UnionValue* union_value2) {
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
  if (methods1.size() == 1u) {
    // Update |union_value2|
    return Unify(NewLiteral(union_value1->value(methods1.front())),
                 union_value2);
  }

  std::vector<ir::Method*> methods2;
  for (auto const method2 : union_value2->methods()) {
    if (Contains(union_value1, union_value2->value(method2)))
      methods2.push_back(method2);
  }
  union_value2->SetMethods(methods2);
  if (methods2.empty())
    return GetEmptyValue();
  if (methods2.size() == 1u)
    return NewLiteral(union_value2->value(methods2.front()));
  return factory()->NewAndValue({union_value1, union_value2});
}

Value* Evaluator::Unify(UnionValue* union_value1, Value* value2) {
  if (auto const and_value2 = value2->as<AndValue>())
    return Unify(union_value1, and_value2);

  if (auto const union_value2 = value2->as<UnionValue>())
    return Unify(union_value1, union_value2);

  NOTREACHED() << "Unify(" << *union_value1 << ", " << value2 << ")";
  return GetEmptyValue();
}

// Unify Variable
Value* Evaluator::Unify(Variable* variable1, Value* value2) {
  if (auto const variable2 = value2->as<Variable>())
    return Unify(variable1, variable2);
  auto const root1 = variable1->Find();
  auto const result = Unify(root1->value(), value2);
  root1->value_ = result;
  return result;
}

Value* Evaluator::Unify(Variable* variable1, Variable* variable2) {
  auto const root1 = variable1->Find();
  auto const root2 = variable2->Find();
  auto const result = Unify(root1->value(), root2->value());
  root1->value_ = result;
  root2->value_ = result;
  Union(root1, root2);
  return result;
}

void Evaluator::Union(Variable* variable1, Variable* variable2) {
  auto const root1 = variable1->Find();
  auto root2 = variable1->Find();
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
