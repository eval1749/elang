// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/instruction_factory.h"

#include "elang/base/zone.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace hir {

InstructionFactory::InstructionFactory(Factory* factory)
    : factory_(factory), type_factory_(new TypeFactory()) {
}

VoidLiteral* InstructionFactory::GetVoidValue() const {
  return GetVoidType()->zero();
}

VoidType* InstructionFactory::GetVoidType() const {
  return types()->GetVoidType();
}

}  // namespace hir
}  // namespace elang
