// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/type_evaluator.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/analyze/type_factory.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// TypeEvaluator::Context
//
struct TypeEvaluator::Context {
  ast::Node* node;
  ts::Value* result;
  ast::Node* user;

  Context(ast::Node* node, ast::Node* user)
      : result(nullptr), node(node), user(user) {}
};

//////////////////////////////////////////////////////////////////////
//
// TypeEvaluator::ScopedContext
//
class TypeEvaluator::ScopedContext {
 public:
  ScopedContext(TypeEvaluator* resolver, ast::Node* node, ast::Node* user);
  ~ScopedContext();

 private:
  Context context_;
  TypeEvaluator* const resolver_;
  Context* const saved_context_;

  DISALLOW_COPY_AND_ASSIGN(ScopedContext);
};

TypeEvaluator::ScopedContext::ScopedContext(TypeEvaluator* resolver,
                                           ast::Node* node,
                                           ast::Node* user)
    : context_(node, user),
      resolver_(resolver),
      saved_context_(resolver->context_) {
  resolver_->context_ = &context_;
}

TypeEvaluator::ScopedContext::~ScopedContext() {
  resolver_->context_ = saved_context_;
}

//////////////////////////////////////////////////////////////////////
//
// TypeEvaluator
//
TypeEvaluator::TypeEvaluator(NameResolver* name_resolver)
    : Analyzer(name_resolver),
      context_(nullptr),
      type_factory_(new ts::Factory()),
      literal_cache_map_(type_factory_->zone()),
      value_cache_map_(type_factory_->zone()) {
}

TypeEvaluator::~TypeEvaluator() {
}

ts::Value* TypeEvaluator::Evaluate(ast::Node* node, ast::Node* user) {
  DCHECK(node);
  DCHECK(user);
  ScopedContext scoped_context(this, node, user);
  DCHECK(context_->result);
  return context_->result;
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

void TypeEvaluator::ProduceResult(ts::Value* result) {
  DCHECK(result);
  DCHECK(context_);
  DCHECK(!context_->result);
  DCHECK(!value_cache_map_.count(context_->node));
  value_cache_map_[context_->node] = result;
  context_->result = result;
}

ts::Value* TypeEvaluator::Union(ts::Value* value1, ts::Value* value2) {
  DCHECK(value1);
  DCHECK(value2);
  return GetAnyValue();
}

// ast::Visitor
void TypeEvaluator::VisitLiteral(ast::Literal* literal) {
  auto const token = literal->token();
  if (token == TokenType::NullLiteral) {
    auto const variable = type_factory_->NewVariable(literal);
    ProduceResult(type_factory_->NewNullValue(variable));
    return;
  }

  // Other than |null| literal, the type of literal is predefined.
  auto const type =
      resolver()->ResolvePredefinedType(token, token->literal_type());
  auto const it = literal_cache_map_.find(type);
  if (it != literal_cache_map_.end()) {
    ProduceResult(it->second);
    return;
  }
  auto const value = type_factory_->NewLiteral(type);
  literal_cache_map_[type] = value;
  ProduceResult(value);
}

}  // namespace compiler
}  // namespace elang
