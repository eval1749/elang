// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/factory.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "elang/base/atomic_string.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/base/zone.h"
#include "elang/hir/intrinsic_names.h"
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
      false_value_(new (zone()) BoolLiteral(types()->bool_type(), false)),
      last_basic_block_id_(0),
      last_function_id_(0),
      last_instruction_id_(0),
      true_value_(new (zone()) BoolLiteral(types()->bool_type(), true)) {
}

Factory::~Factory() {
}

AtomicString* Factory::intrinsic_name(IntrinsicName name) {
  static const base::char16* names[] = {
#define V(name) L## #name,
      FOR_EACH_INTRINSIC_NAME(V)
#undef V
  };
  return NewAtomicString(names[static_cast<size_t>(name)]);
}

BasicBlock* Factory::NewBasicBlock() {
  return new (zone()) BasicBlock(this);
}

AtomicString* Factory::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

Value* Factory::NewBoolLiteral(bool data) {
  return data ? true_value_ : false_value_;
}

CharLiteral* Factory::NewCharLiteral(base::char16 data) {
  return new (zone()) CharLiteral(types()->char_type(), data);
}

Float32Literal* Factory::NewFloat32Literal(float32_t data) {
  return new (zone()) Float32Literal(types()->float32_type(), data);
}

Float64Literal* Factory::NewFloat64Literal(float64_t data) {
  return new (zone()) Float64Literal(types()->float64_type(), data);
}

Function* Factory::NewFunction(FunctionType* type) {
  return new (zone()) Function(this, type, ++last_function_id_);
}

Int16Literal* Factory::NewInt16Literal(int16_t data) {
  return new (zone()) Int16Literal(types()->int16_type(), data);
}

Int32Literal* Factory::NewInt32Literal(int32_t data) {
  return new (zone()) Int32Literal(types()->int32_type(), data);
}

Int64Literal* Factory::NewInt64Literal(int64_t data) {
  return new (zone()) Int64Literal(types()->int64_type(), data);
}

Int8Literal* Factory::NewInt8Literal(int8_t data) {
  return new (zone()) Int8Literal(types()->int8_type(), data);
}

Reference* Factory::NewReference(Type* type, AtomicString* name) {
  // Check |name| is created from |atomic_string_factory_|.
  DCHECK_EQ(name, NewAtomicString(name->string()));
  auto const it = reference_cache_.find(name);
  if (it != reference_cache_.end())
    return it->second;
  auto const new_reference = new (zone()) Reference(type, name);
  reference_cache_[name] = new_reference;
  return new_reference;
}

base::StringPiece16 Factory::NewString(base::StringPiece16 string) {
  return atomic_string_factory_->NewString(string);
}

StringLiteral* Factory::NewStringLiteral(base::StringPiece16 data) {
  return new (zone()) StringLiteral(types()->string_type(), NewString(data));
}

UInt16Literal* Factory::NewUInt16Literal(uint16_t data) {
  return new (zone()) UInt16Literal(types()->uint16_type(), data);
}

UInt32Literal* Factory::NewUInt32Literal(uint32_t data) {
  return new (zone()) UInt32Literal(types()->uint32_type(), data);
}

UInt64Literal* Factory::NewUInt64Literal(uint64_t data) {
  return new (zone()) UInt64Literal(types()->uint64_type(), data);
}

UInt8Literal* Factory::NewUInt8Literal(uint8_t data) {
  return new (zone()) UInt8Literal(types()->uint8_type(), data);
}

int Factory::NextBasicBlockId() {
  return ++last_basic_block_id_;
}

int Factory::NextInstructionId() {
  return ++last_instruction_id_;
}

}  // namespace hir
}  // namespace elang
