// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FACTORY_H_
#define ELANG_HIR_FACTORY_H_

#include <memory>

#include "elang/hir/hir_export.h"
#include "elang/hir/instruction_factory.h"
#include "elang/hir/type_factory.h"

namespace elang {
class Zone;
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class ELANG_HIR_EXPORT Factory final : public InstructionFactory {
 public:
  Factory();
  ~Factory();

  Zone* zone() const { return zone_.get(); }

  BasicBlock* NewBasicBlock();
  Function* NewFunction(FunctionType* function_type);

  int NextBasicBlockId();
  int NextInstructionId();

 private:
  int last_basic_block_id_;
  int last_instruction_id_;
  const std::unique_ptr<Zone> zone_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FACTORY_H_
