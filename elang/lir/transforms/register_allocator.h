// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_
#define ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/instruction_visitor.h"

namespace elang {

template <typename Graph>
class DominatorTree;

namespace lir {

class BasicBlock;
class Factory;
class Function;
class RegisterAllocationMap;
class UseDefList;

//////////////////////////////////////////////////////////////////////
//
// RegisterAllocator
//
class ELANG_LIR_EXPORT RegisterAllocator final : public FunctionPass,
                                                 public InstructionVisitor {
 public:
  RegisterAllocator(Factory* factory,
                    Function* function,
                    RegisterAllocationMap* allocation_map,
                    const RegisterUsageTracker& usage_tracker);
  ~RegisterAllocator();

 private:
  class LocalAllocator;
  class LocalAllocationMap;

  // Pass
  base::StringPiece name() const final;

  int NextUseOf(Instruction* instr, Value input) const;

  // Function Pass
  void RunOnFunction() final;

  void ProcessBlock(BasicBlock* block);
  void ProcessInputOperand(Instruction* instruction, Value input, int position);
  void ProcessOutputOperand(Instruction* instruction, Value output);

  RegisterAllocationMap* const allocation_map_;
  DominatorTree<Function>* const dominator_tree_;
  const std::unique_ptr<LocalAllocator> local_allocator_;
  const RegisterUsageTracker& usage_tracker_;

  DISALLOW_COPY_AND_ASSIGN(RegisterAllocator);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_REGISTER_ALLOCATOR_H_
