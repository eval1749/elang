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
      false_value_(bool_type()->as<BoolType>()->NewLiteral(zone(), false)),
      last_basic_block_id_(0),
      last_function_id_(0),
      last_instruction_id_(0),
      true_value_(bool_type()->as<BoolType>()->NewLiteral(zone(), true)),
      void_value_(void_type()->default_value()) {
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

Function* Factory::NewFunction(FunctionType* type) {
  return new (zone()) Function(this, type, ++last_function_id_);
}

#define V(Name, name, data_type, ...)                                 \
  Value* Factory::New##Name##Literal(data_type data) {                \
    return name##_type()->as<Name##Type>()->NewLiteral(zone(), data); \
  }
FOR_EACH_HIR_PRIMITIVE_VALUE_TYPE(V)
#undef V

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

SizeOf* Factory::NewSizeOf(Type* type) {
  auto const it = sizeof_cache_.find(type);
  if (it != sizeof_cache_.end())
    return it->second;
  auto const new_sizeof = new (zone()) SizeOf(types()->uintptr_type(), type);
  sizeof_cache_[type] = new_sizeof;
  return new_sizeof;
}

base::StringPiece16 Factory::NewString(base::StringPiece16 string) {
  return atomic_string_factory_->NewString(string);
}

StringLiteral* Factory::NewStringLiteral(base::StringPiece16 data) {
  return new (zone()) StringLiteral(string_type(), NewString(data));
}

int Factory::NextBasicBlockId() {
  return ++last_basic_block_id_;
}

int Factory::NextInstructionId() {
  return ++last_instruction_id_;
}

}  // namespace hir
}  // namespace elang
