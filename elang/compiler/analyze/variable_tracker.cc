// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/variable_tracker.h"

#include "base/logging.h"
#include "elang/base/zone.h"
#include "elang/compiler/analyze/type_unifyer.h"
#include "elang/compiler/analyze/type_values.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// VariableTracker::TrackingData
//
struct VariableTracker::TrackingData : ZoneAllocated {
  int local_get_count;
  int local_set_count;
  int nonlocal_get_count;
  int nonlocal_set_count;
  ts::Value* value;

  TrackingData();
  ir::StorageClass ComputeStorageClass() const;
};

VariableTracker::TrackingData::TrackingData()
    : local_get_count(0),
      local_set_count(0),
      nonlocal_get_count(0),
      nonlocal_set_count(0),
      value(nullptr) {
}

ir::StorageClass VariableTracker::TrackingData::ComputeStorageClass() const {
  if (nonlocal_set_count)
    return ir::StorageClass::Heap;
  if (nonlocal_get_count || local_set_count)
    return ir::StorageClass::Stack;
  if (local_get_count)
    return ir::StorageClass::Register;
  // Maybe variable used as |static_cast<void>(x)|.
  return ir::StorageClass::Void;
}

//////////////////////////////////////////////////////////////////////
//
// VariableTracker
//
VariableTracker::VariableTracker(CompilationSession* session,
                                 Zone* zone,
                                 ast::Method* context_method)
    : CompilationSessionUser(session),
      context_method_(context_method),
      zone_(zone) {
}

VariableTracker::~VariableTracker() {
}

void VariableTracker::Finish(ir::Factory* factory, ts::Factory* type_factory) {
  ts::TypeUnifyer evaluator(type_factory);
  for (auto variable_data : variable_map_) {
    auto const variable = variable_data.first;
    auto const data = variable_data.second;
    auto const literal = evaluator.Evaluate(data->value)->as<ts::Literal>();
    if (!literal) {
      session()->AddError(ErrorCode::TypeResolverVariableNotResolved,
                          variable->name());
      continue;
    }
    semantics()->SetValue(
        variable, factory->NewVariable(literal->value(),
                                       data->ComputeStorageClass(), variable));
  }
}

ts::Value* VariableTracker::RecordGet(ast::NamedNode* variable) {
  // TODO(eval1749) NYI nonlocal reference of variable.
  auto const it = variable_map_.find(variable);
  DCHECK(it != variable_map_.end());
  auto const data = it->second;
  ++data->local_get_count;
  return data->value;
}

void VariableTracker::RecordSet(ast::NamedNode* variable) {
  // TODO(eval1749) NYI nonlocal reference of variable.
  auto const it = variable_map_.find(variable);
  DCHECK(it != variable_map_.end());
  ++it->second->local_set_count;
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
