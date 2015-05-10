// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/type_factory.h"
#include "elang/compiler/analysis/type_values.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {
namespace ts {

//////////////////////////////////////////////////////////////////////
//
// The Type Factory
//
Factory::Factory(CompilationSession* session, Zone* zone)
    : CompilationSessionUser(session),
      ZoneUser(zone),
      any_value_(new (zone) AnyValue),
      empty_value_(new (zone) EmptyValue()),
      literal_cache_map_(zone),
      null_value_cache_map_(zone),
      bool_value_(NewPredefinedValue(PredefinedName::Bool)),
      float32_value_(NewPredefinedValue(PredefinedName::Float32)),
      float64_value_(NewPredefinedValue(PredefinedName::Float64)),
      int16_value_(NewPredefinedValue(PredefinedName::Int16)),
      int32_value_(NewPredefinedValue(PredefinedName::Int32)),
      int64_value_(NewPredefinedValue(PredefinedName::Int64)),
      int8_value_(NewPredefinedValue(PredefinedName::Int8)),
      uint16_value_(NewPredefinedValue(PredefinedName::UInt16)),
      uint32_value_(NewPredefinedValue(PredefinedName::UInt32)),
      uint64_value_(NewPredefinedValue(PredefinedName::UInt64)),
      uint8_value_(NewPredefinedValue(PredefinedName::UInt8)) {
}

Factory::~Factory() {
}

AndValue* Factory::NewAndValue(const std::vector<UnionValue*>& union_values) {
  return new (zone()) AndValue(zone(), union_values);
}

Argument* Factory::NewArgument(CallValue* call_value, int position) {
  return new (zone()) Argument(call_value, position);
}

CallValue* Factory::NewCallValue(ast::Call* ast_call) {
  return new (zone()) CallValue(zone(), ast_call);
}

Value* Factory::NewInvalidValue(ast::Node* node) {
  return new (zone()) InvalidValue(node);
}

Value* Factory::NewLiteral(sm::Type* type) {
  auto const it = literal_cache_map_.find(type);
  if (it != literal_cache_map_.end())
    return it->second;
  auto const value = new (zone()) Literal(type);
  literal_cache_map_[type] = value;
  return value;
}

Value* Factory::NewNullValue(Value* base_value) {
  auto const it = null_value_cache_map_.find(base_value);
  if (it != null_value_cache_map_.end())
    return it->second;
  auto const value = new (zone()) NullValue(base_value);
  null_value_cache_map_[base_value] = value;
  return value;
}

Value* Factory::NewPredefinedValue(PredefinedName name) {
  return NewLiteral(session()->PredefinedTypeOf(name));
}

Variable* Factory::NewVariable(ast::Node* node, Value* value) {
  return new (zone()) Variable(node, value);
}

}  // namespace ts
}  // namespace compiler
}  // namespace elang
