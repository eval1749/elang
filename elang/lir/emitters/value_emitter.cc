// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/emitters/value_emitter.h"

#include "base/logging.h"
#include "elang/api/machine_code_builder.h"
#include "elang/lir/factory.h"
#include "elang/lir/literals.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// ValueEmitter
//
ValueEmitter::ValueEmitter(Factory* factory, api::MachineCodeBuilder* builder)
    : builder_(builder), code_offset_(-1), factory_(factory) {
}

void ValueEmitter::Emit(int code_offset, Value value) {
  DCHECK_GE(code_offset, 0);
  DCHECK_EQ(code_offset_, -1);
  code_offset_ = code_offset;
  switch (value.kind) {
    case Value::Kind::Immediate:
      builder_->SetInt32(code_offset, value.data);
      break;
    case Value::Kind::Literal:
      factory_->GetLiteral(value)->Accept(this);
      break;
    default:
      NOTREACHED() << "Unexpected value: " << value;
      break;
  }
  code_offset_ = -1;
}

// LiteralVisitor
void ValueEmitter::VisitBasicBlock(BasicBlock* literal) {
  DCHECK(literal);
  NOTREACHED() << "NYI: BasicBlock literal in code emitter";
}

void ValueEmitter::VisitFunction(Function* literal) {
  DCHECK(literal);
  NOTREACHED() << "NYI: Function literal in code emitter";
}

void ValueEmitter::VisitFloat32Literal(Float32Literal* literal) {
  builder_->SetFloat32(code_offset_, literal->data());
}

void ValueEmitter::VisitFloat64Literal(Float64Literal* literal) {
  builder_->SetFloat64(code_offset_, literal->data());
}

void ValueEmitter::VisitInt32Literal(Int32Literal* literal) {
  builder_->SetInt32(code_offset_, literal->data());
}

void ValueEmitter::VisitInt64Literal(Int64Literal* literal) {
  builder_->SetInt64(code_offset_, literal->data());
}

void ValueEmitter::VisitStringLiteral(StringLiteral* literal) {
  builder_->SetString(code_offset_, literal->data());
}

}  // namespace lir
}  // namespace elang
