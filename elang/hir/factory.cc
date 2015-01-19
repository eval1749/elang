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
#include "elang/hir/value_visitor.h"

namespace elang {
namespace hir {

#define V(Name, ...) \
  void ValueVisitor::Visit##Name(Name* value) { DoDefaultVisit(value); }
FOR_EACH_HIR_VALUE(V)
#undef V

void ValueVisitor::VisitInstruction(Instruction* value) {
  DCHECK(value);
}

void ValueVisitor::DoDefaultVisit(Value* value) {
  DCHECK(value);
}

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory(const FactoryConfig& config)
    : InstructionFactory(this, config),
      atomic_string_factory_(config.atomic_string_factory),
      config_(config),
      false_literal_(new (zone()) BoolLiteral(types()->GetBoolType(), false)),
      last_basic_block_id_(0),
      last_instruction_id_(0),
      true_literal_(new (zone()) BoolLiteral(types()->GetBoolType(), true)) {
}

Factory::~Factory() {
}

BasicBlock* Factory::NewBasicBlock() {
  return new (zone()) BasicBlock(this);
}

AtomicString* Factory::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

BoolLiteral* Factory::NewBoolLiteral(bool data) {
  return data ? true_literal_ : false_literal_;
}

CharLiteral* Factory::NewCharLiteral(base::char16 data) {
  return new (zone()) CharLiteral(types()->GetCharType(), data);
}

Float32Literal* Factory::NewFloat32Literal(float32_t data) {
  return new (zone()) Float32Literal(types()->GetFloat32Type(), data);
}

Float64Literal* Factory::NewFloat64Literal(float64_t data) {
  return new (zone()) Float64Literal(types()->GetFloat64Type(), data);
}

Function* Factory::NewFunction(FunctionType* type) {
  return new (zone()) Function(this, type);
}

Int16Literal* Factory::NewInt16Literal(int16_t data) {
  return new (zone()) Int16Literal(types()->GetInt16Type(), data);
}

Int32Literal* Factory::NewInt32Literal(int32_t data) {
  return new (zone()) Int32Literal(types()->GetInt32Type(), data);
}

Int64Literal* Factory::NewInt64Literal(int64_t data) {
  return new (zone()) Int64Literal(types()->GetInt64Type(), data);
}

Int8Literal* Factory::NewInt8Literal(int8_t data) {
  return new (zone()) Int8Literal(types()->GetInt8Type(), data);
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

UInt16Literal* Factory::NewUInt16Literal(uint16_t data) {
  return new (zone()) UInt16Literal(types()->GetUInt16Type(), data);
}

UInt32Literal* Factory::NewUInt32Literal(uint32_t data) {
  return new (zone()) UInt32Literal(types()->GetUInt32Type(), data);
}

UInt64Literal* Factory::NewUInt64Literal(uint64_t data) {
  return new (zone()) UInt64Literal(types()->GetUInt64Type(), data);
}

UInt8Literal* Factory::NewUInt8Literal(uint8_t data) {
  return new (zone()) UInt8Literal(types()->GetUInt8Type(), data);
}

int Factory::NextBasicBlockId() {
  return ++last_basic_block_id_;
}

int Factory::NextInstructionId() {
  return ++last_instruction_id_;
}

}  // namespace hir
}  // namespace elang
