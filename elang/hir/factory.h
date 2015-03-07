// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FACTORY_H_
#define ELANG_HIR_FACTORY_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/hir/factory_config.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/instruction_factory.h"

namespace elang {
namespace hir {

enum class IntrinsicName;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class ELANG_HIR_EXPORT Factory final : public InstructionFactory {
 public:
  explicit Factory(const FactoryConfig& config);
  ~Factory();

  const FactoryConfig& config() const { return config_; }
  Value* false_value() const { return false_value_; }
  AtomicString* intrinsic_name(IntrinsicName name);
  Value* true_value() const { return true_value_; }
  Value* void_value() const { return void_value_; }

  AtomicString* NewAtomicString(base::StringPiece16 string);
  BasicBlock* NewBasicBlock();
  Function* NewFunction(FunctionType* function_type);
  Reference* NewReference(Type* type, AtomicString* name);
  SizeOf* NewSizeOf(Type* type);
  base::StringPiece16 NewString(base::StringPiece16 string_piece);
  StringLiteral* NewStringLiteral(base::StringPiece16 data);

#define V(Name, name, data_type, ...) \
  Value* Factory::New##Name##Literal(data_type data);
  FOR_EACH_HIR_PRIMITIVE_VALUE_TYPE(V)
#undef V

  int NextBasicBlockId();
  int NextInstructionId();

 private:
  AtomicStringFactory* const atomic_string_factory_;
  const FactoryConfig config_;
  Value* const false_value_;
  int last_basic_block_id_;
  int last_function_id_;
  int last_instruction_id_;
  std::unordered_map<AtomicString*, Reference*> reference_cache_;
  std::unordered_map<Type*, SizeOf*> sizeof_cache_;
  Value* const true_value_;
  Value* const void_value_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FACTORY_H_
