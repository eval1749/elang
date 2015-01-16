// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/type_evaluator.h"

#include "base/logging.h"
#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_values.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// TypeEvaluator
//
TypeEvaluator::TypeEvaluator(NameResolver* name_resolver)
    : Analyzer(name_resolver), type_factory_(new ts::Factory()) {
}

TypeEvaluator::~TypeEvaluator() {
}

ts::Value* TypeEvaluator::Evaluate(ast::Node* node, ast::Node* user) {
  DCHECK(node);
  DCHECK(user);
  return GetEmptyValue();
}

ts::Value* TypeEvaluator::GetAnyValue() {
  return type_factory_->GetAnyValue();
}

ts::Value* TypeEvaluator::GetEmptyValue() {
  return type_factory_->GetEmptyValue();
}

ts::Value* TypeEvaluator::Intersect(ts::Value* value1, ts::Value* value2) {
  DCHECK(value1);
  DCHECK(value2);
  return GetEmptyValue();
}

ts::Value* TypeEvaluator::NewInvalidValue(ast::Node* node) {
  return type_factory_->NewInvalidValue(node);
}

ts::Value* TypeEvaluator::Union(ts::Value* value1, ts::Value* value2) {
  DCHECK(value1);
  DCHECK(value2);
  return GetAnyValue();
}

}  // namespace compiler
}  // namespace elang
