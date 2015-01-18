// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FACTORY_H_
#define ELANG_HIR_FACTORY_H_

#include <string>
#include <memory>

#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/hir/factory_config.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/instruction_factory.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class ELANG_HIR_EXPORT Factory final : public InstructionFactory {
 public:
  explicit Factory(const FactoryConfig& config);
  ~Factory();

  BasicBlock* NewBasicBlock();
  AtomicString* NewAtomicString(base::StringPiece16 string);
  Function* NewFunction(FunctionType* function_type);
  Reference* NewReference(Type* type, base::StringPiece16 name);
  base::StringPiece16 NewString(base::StringPiece16 string_piece);
  StringLiteral* NewStringLiteral(base::StringPiece16 data);

  int NextBasicBlockId();
  int NextInstructionId();

 private:
  AtomicStringFactory* const atomic_string_factory_;
  const FactoryConfig config_;
  int last_basic_block_id_;
  int last_instruction_id_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FACTORY_H_
