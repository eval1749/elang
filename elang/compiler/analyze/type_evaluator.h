// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_
#define ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_

#include <memory>

#include "base/macros.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/compiler/analyze/analyzer.h"
#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace compiler {
namespace ts {
class Factory;
class Value;
}

//////////////////////////////////////////////////////////////////////
//
// TypeEvaluator
//
class TypeEvaluator final : public Analyzer, public ast::Visitor {
 public:
  explicit TypeEvaluator(NameResolver* name_resolver);
  ~TypeEvaluator();

  // Evaluate type value of |node| for |user|.
  ts::Value* Evaluate(ast::Node* node, ast::Node* user);
  // Get singleton |AnyValue| instance.
  ts::Value* GetAnyValue();
  // Get singleton |EmpyValue| instance.
  ts::Value* GetEmptyValue();
  // Unify type value of |expression| with |value|.
  ts::Value* Intersect(ts::Value* value1, ts::Value* value2);
  ts::Value* NewInvalidValue(ast::Node* node);
  ts::Value* NewLiteral(ir::Type* literal);
  ts::Value* Union(ts::Value* value1, ts::Value* value2);

 private:
  struct Context;
  class ScopedContext;

  void ProduceResult(ts::Value* value);

  // ast::Visitor
  void VisitLiteral(ast::Literal* node) final;

  Context* context_;

  std::unique_ptr<ts::Factory> type_factory_;
  // Zone allocated objects must be constructed after |ts::Factory|.
  ZoneUnorderedMap<ir::Type*, ts::Value*> literal_cache_map_;
  ZoneUnorderedMap<ast::Node*, ts::Value*> value_cache_map_;

  DISALLOW_COPY_AND_ASSIGN(TypeEvaluator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_
