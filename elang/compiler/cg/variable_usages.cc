// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/variable_usages.h"

#include "elang/hir/instructions.h"
#include "elang/hir/types.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// VariableUsages::Data
//
VariableUsages::Data::Data(hir::Instruction* home)
    : home_(home),
      owner_(home->function()),
      type_(home->type()->as<hir::PointerType>()->pointee()),
      used_in_(UsedIn::SingleBlock) {
}

//////////////////////////////////////////////////////////////////////
//
// VariableUsages::PerFunctionData
//
VariableUsages::PerFunctionData::PerFunctionData(Zone* zone)
    : local_variables(zone), non_local_reads(zone), non_local_writes(zone) {
}

//////////////////////////////////////////////////////////////////////
//
// VariableUsages
//
VariableUsages::VariableUsages(Zone* zone)
    : function_map_(zone), variable_map_(zone) {
}

VariableUsages::Data* VariableUsages::data_for(hir::Instruction* home) const {
  auto const it = variable_map_.find(home);
  return it == variable_map_.end() ? nullptr : it->second;
}

const ZoneVector<VariableUsages::Data*>& VariableUsages::local_variables_of(
    hir::Function* function) const {
  auto const it = function_map_.find(function);
  DCHECK(it != function_map_.end());
  return it->second->local_variables;
}

bool VariableUsages::IsAliveAt(hir::Instruction* home,
                               hir::BasicBlock* block) const {
  auto const data = data_for(home);
  if (!data)
    return false;
  return data->used_in_ != Data::UsedIn::SingleBlock;
}

}  // namespace compiler
}  // namespace elang
