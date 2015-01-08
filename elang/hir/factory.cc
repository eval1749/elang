// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/factory.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "elang/base/zone.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory()
    : InstructionFactory(this),
      last_basic_block_id_(0),
      last_instruction_id_(0),
      zone_(new Zone()) {
}

Factory::~Factory() {
}

BasicBlock* Factory::NewBasicBlock() {
  return new (zone()) BasicBlock(this);
}

Function* Factory::NewFunction(FunctionType* type) {
  return new (zone()) Function(this, type);
}

int Factory::NextBasicBlockId() {
  return ++last_basic_block_id_;
}

int Factory::NextInstructionId() {
  return ++last_instruction_id_;
}

}  // namespace hir
}  // namespace elang
