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

UseDefListBuilder::UseDefListBuilder() : use_def_list_(new UseDefList()) {
}

UseDefListBuilder::~UseDefListBuilder() {
}

void UseDefListBuilder::AddUser(Value value, Instruction* user) {
  if (!value.is_virtual())
    return;
  auto const it = use_def_list_->map_.find(value);
  DCHECK(it != use_def_list_->map_.end());
  auto const users = it->second;
  if (!users->empty() && users->users_.back() == user)
    return;
  users->users_.push_back(user);
}

void UseDefListBuilder::Assign(Value value) {
  if (!value.is_virtual())
    return;
  DCHECK(!use_def_list_->map_.count(value));
  auto const zone = use_def_list_->zone();
  use_def_list_->map_[value] = new (zone) UseDefList::Users(zone);
}

std::unique_ptr<UseDefList> UseDefListBuilder::Build(Function* function) {
  DCHECK(use_def_list_);
  auto use_def_list = std::make_unique<UseDefList>();
  for (auto const block : function->basic_blocks()) {
    for (auto const phi : block->phi_instructions())
      Assign(phi->output(0));
    for (auto const instr : block->instructions()) {
      for (auto const input : instr->inputs())
        AddUser(input, instr);
      for (auto const output : instr->outputs())
        Assign(output);
    }
  }
  return std::move(use_def_list_);
}

}  // namespace lir
}  // namespace elang
