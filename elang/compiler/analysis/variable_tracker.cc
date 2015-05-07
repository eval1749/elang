// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/variable_tracker.h"

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/compiler/analysis/type_evaluator.h"
#include "elang/compiler/analysis/type_values.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// VariableTracker::TrackingData
//
struct VariableTracker::TrackingData : ZoneAllocated {
  int heap_get_count;
  int heap_set_count;
  int local_get_count;
  int local_set_count;
  int non_local_get_count;
  int non_local_set_count;
  ts::Value* value;

  TrackingData();
  sm::StorageClass ComputeStorageClass() const;
};

VariableTracker::TrackingData::TrackingData()
    : heap_get_count(0),
      heap_set_count(0),
      local_get_count(0),
      local_set_count(0),
      non_local_get_count(0),
      non_local_set_count(0),
      value(nullptr) {
}

sm::StorageClass VariableTracker::TrackingData::ComputeStorageClass() const {
  if (non_local_set_count)
    return sm::StorageClass::Heap;
  if (non_local_set_count)
    return sm::StorageClass::NonLocal;
  if (local_set_count)
    return sm::StorageClass::Local;
  if (heap_get_count || local_get_count || non_local_get_count)
    return sm::StorageClass::ReadOnly;
  // Maybe variable used as |static_cast<void>(x)|.
  return sm::StorageClass::Void;
}

//////////////////////////////////////////////////////////////////////
//
// VariableTracker
//
VariableTracker::VariableTracker(CompilationSession* session,
                                 Zone* zone,
                                 ast::Method* context_method)
    : CompilationSessionUser(session),
      ZoneUser(zone),
      context_method_(context_method) {
}

VariableTracker::~VariableTracker() {
}

void VariableTracker::Finish(sm::Factory* factory, ts::Factory* type_factory) {
  ts::Evaluator evaluator(type_factory);
  for (auto variable_data : variable_map_) {
    auto const variable = variable_data.first;
    auto const data = variable_data.second;
    auto const literal = evaluator.Evaluate(data->value)->as<ts::Literal>();
    if (!literal) {
      session()->AddError(ErrorCode::TypeResolverVariableNotResolved,
                          variable->name());
      continue;
    }
    semantics()->SetSemanticOf(
        variable, factory->NewVariable(literal->value(),
                                       data->ComputeStorageClass(), variable));
  }
}

ts::Value* VariableTracker::RecordGet(ast::NamedNode* variable) {
  // TODO(eval1749) NYI non_local reference of variable.
  auto const it = variable_map_.find(variable);
  DCHECK(it != variable_map_.end());
  auto const data = it->second;
  ++data->local_get_count;
  return data->value;
}

ts::Value* VariableTracker::RecordSet(ast::NamedNode* variable) {
  // TODO(eval1749) NYI non_local reference of variable.
  auto const it = variable_map_.find(variable);
  DCHECK(it != variable_map_.end());
  auto const data = it->second;
  ++data->local_set_count;
  return data->value;
}

void VariableTracker::RegisterVariable(ast::NamedNode* variable,
                                       ts::Value* value) {
  DCHECK(!variable_map_.count(variable));
  auto const data = new (zone()) TrackingData();
  data->value = value;
  variable_map_[variable] = data;
}

}  // namespace compiler
}  // namespace elang
