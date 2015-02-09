// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/analysis/use_def_list_builder.h"

#include "base/logging.h"
#include "elang/lir/analysis/use_def_list.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

UseDefListBuilder::UseDefListBuilder(Function* function)
    : function_(function) {
}

UseDefListBuilder::~UseDefListBuilder() {
}

void UseDefListBuilder::AddUser(UseDefList* use_def_list,
                                Value value,
                                Instruction* user) {
  if (!value.is_virtual())
    return;
  auto const it = use_def_list->map_.find(value);
  DCHECK(it != use_def_list->map_.end());
  auto const users = it->second;
  if (!users->empty() && users->users_.back() == user)
    return;
  users->users_.push_back(user);
}

void UseDefListBuilder::Assign(UseDefList* use_def_list, Value value) {
  if (!value.is_virtual())
    return;
  DCHECK(!use_def_list->map_.count(value));
  auto const zone = use_def_list->zone();
  use_def_list->map_[value] = new (zone) UseDefList::Users(zone);
}

UseDefList UseDefListBuilder::Build() {
  UseDefList use_def_list;
  for (auto const block : function_->basic_blocks()) {
    for (auto const phi : block->phi_instructions())
      Assign(&use_def_list, phi->output(0));
    for (auto const instr : block->instructions()) {
      for (auto const input : instr->inputs())
        AddUser(&use_def_list, input, instr);
      for (auto const output : instr->outputs())
        Assign(&use_def_list, output);
    }
  }
  return std::move(use_def_list);
}

}  // namespace lir
}  // namespace elang
