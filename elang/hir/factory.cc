// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/factory.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/base/zone.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory(const FactoryConfig& config)
    : InstructionFactory(this, config),
      atomic_string_factory_(config.atomic_string_factory),
      config_(config),
      last_basic_block_id_(0),
      last_instruction_id_(0) {
}

Factory::~Factory() {
}

BasicBlock* Factory::NewBasicBlock() {
  return new (zone()) BasicBlock(this);
}

AtomicString* Factory::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

Function* Factory::NewFunction(FunctionType* type) {
  return new (zone()) Function(this, type);
}

Reference* Factory::NewReference(Type* type, base::StringPiece16 name) {
  return new (zone()) Reference(type, NewString(name));
}

base::StringPiece16 Factory::NewString(base::StringPiece16 string) {
  return atomic_string_factory_->NewString(string);
}

StringLiteral* Factory::NewStringLiteral(base::StringPiece16 data) {
  return new (zone()) StringLiteral(types()->GetStringType(), NewString(data));
}

int Factory::NextBasicBlockId() {
  return ++last_basic_block_id_;
}

int Factory::NextInstructionId() {
  return ++last_instruction_id_;
}

}  // namespace hir
}  // namespace elang
