// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_
#define ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_

#include <memory>

#include "base/macros.h"
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
  ts::Value* GetAnyValue();
  ts::Value* GetEmptyValue();
  // Unify type value of |expression| with |value|.
  ts::Value* Intersect(ts::Value* value1, ts::Value* value2);
  ts::Value* NewInvalidValue(ast::Node* node);
  ts::Value* Union(ts::Value* value1, ts::Value* value2);

 private:
  std::unique_ptr<ts::Factory> type_factory_;

  DISALLOW_COPY_AND_ASSIGN(TypeEvaluator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_EVALUATOR_H_
