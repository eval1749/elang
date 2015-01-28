// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/variable_analyzer.h"

#include "elang/compiler/cg/variable_usages.h"
#include "elang/hir/instructions.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// VariableAnalyzer
//
VariableAnalyzer::VariableAnalyzer(Zone* result_zone)
    : did_analyzed_(false),
      result_(new (result_zone) VariableUsages(result_zone)),
      result_zone_(result_zone) {
}

VariableAnalyzer::~VariableAnalyzer() {
  for (auto it : function_map_) {
    delete it.second;
  }
}

VariableUsages* VariableAnalyzer::Analyze() {
  if (did_analyzed_)
    return result_;
  for (const auto& function_data : function_map_) {
    auto const result_function_data =
        result_->function_map_[function_data.first];

    result_function_data->non_local_reads.reserve(
        function_data.second->non_local_reads.size());
    result_function_data->non_local_reads.resize(0);
    for (auto home : function_data.second->non_local_reads)
      result_function_data->non_local_reads.push_back(result_->data_for(home));

    result_function_data->non_local_writes.reserve(
        function_data.second->non_local_writes.size());
    result_function_data->non_local_writes.resize(0);
    for (auto home : function_data.second->non_local_writes)
      result_function_data->non_local_writes.push_back(result_->data_for(home));
  }
  did_analyzed_ = true;
  return result_;
}

void VariableAnalyzer::DidSetVariable(hir::Instruction* home,
                                      hir::BasicBlock* block) {
  UpdateVariableUsage(home, block);
  auto const per_function_data = function_map_[block->function()];
  per_function_data->non_local_writes.insert(home);
  per_function_data->non_local_reads.erase(home);
}

void VariableAnalyzer::DidUseVariable(hir::Instruction* home,
                                      hir::BasicBlock* block) {
  UpdateVariableUsage(home, block);
  auto const per_function_data = function_map_[block->function()];
  if (per_function_data->non_local_writes.count(home))
    return;
  per_function_data->non_local_reads.insert(home);
}

void VariableAnalyzer::RegisterFunction(hir::Function* function) {
  DCHECK(!function_map_.count(function));
  result_->function_map_[function] =
      new (result_zone_) VariableUsages::PerFunctionData(result_zone_);
  function_map_[function] = new PerFunctionData();
}

void VariableAnalyzer::RegisterVariable(hir::Instruction* home) {
  DCHECK(!result_->variable_map_.count(home));
  auto const data = new (result_zone_) VariableUsages::Data(home);
  result_->variable_map_[home] = data;
  auto const result_function_data = result_->function_map_[home->function()];
  result_function_data->local_variables.push_back(data);
}

void VariableAnalyzer::UpdateVariableUsage(hir::Instruction* home,
                                           hir::BasicBlock* block) {
  DCHECK(home->basic_block());
  if (home->basic_block() == block)
    return;
  auto const it = result_->variable_map_.find(home);
  DCHECK(it != result_->variable_map_.end());
  if (home->function() == block->function()) {
    if (it->second->used_in_ == VariableUsages::Data::UsedIn::SingleBlock)
      it->second->used_in_ = VariableUsages::Data::UsedIn::MultipleBlocks;
    return;
  }
  it->second->used_in_ = VariableUsages::Data::UsedIn::NonLocalBlocks;
}

}  // namespace compiler
}  // namespace elang
