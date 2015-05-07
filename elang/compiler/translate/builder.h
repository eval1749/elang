// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TRANSLATE_BUILDER_H_
#define ELANG_COMPILER_TRANSLATE_BUILDER_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "elang/base/zone_owner.h"
#include "elang/optimizer/factory_user.h"
#include "elang/optimizer/nodes_forward.h"

namespace elang {
namespace optimizer {
class Factory;
class Editor;
class Node;
class TypeFactory;
}
namespace compiler {

namespace sm {
class Variable;
}

namespace ir = optimizer;

class IrTypeMapper;

//////////////////////////////////////////////////////////////////////
//
// Builder
//
class Builder final : public ZoneOwner {
 public:
  Builder(ir::Factory* factory, ir::Function* function);
  ~Builder();

  bool has_control() const { return basic_block_ != nullptr; }

  // Control flow
  ir::Control* EndBlockWithBranch(ir::Data* condition);
  ir::Control* EndBlockWithJump(ir::Control* target);
  void EndBlockWithRet(ir::Data* data);
  void EndLoopBlock(ir::Data* condition,
                    ir::Control* true_target,
                    ir::Control* false_target);
  ir::PhiOwnerNode* NewMergeBlock();
  void StartIfBlock(ir::Control* control);
  void StartMergeBlock(ir::PhiOwnerNode* control);
  ir::Control* StartDoLoop(ir::LoopNode* loop_block);
  void StartWhileLoop(ir::Data* condition,
                      ir::LoopNode* loop_block,
                      ir::PhiOwnerNode* break_block);

  // Effect consumer/producer
  ir::Data* Call(ir::Data* callee, ir::Node* arguments);
  ir::Data* NewLoad(ir::Data* anchor, ir::Data* pointer);
  void NewStore(ir::Data* anchor, ir::Data* pointer, ir::Data* new_value);

  // Variable management
  void AssignVariable(sm::Variable* variable, ir::Data* value);
  void BindVariable(sm::Variable* variable, ir::Data* value);
  void EndVariableScope();
  void StartVariableScope();
  void UnbindVariable(sm::Variable* variable);
  ir::Data* VariableValueOf(sm::Variable* variable) const;

  ir::Data* ParameterAt(size_t index);

 private:
  class BasicBlock;
  class VariableTracker;

  BasicBlock* BasicBlockOf(ir::Control* control);
  void EndBlock(ir::Control* control);
  BasicBlock* NewBasicBlock(ir::Control* control, ir::Effect* effect);
  void PopulatePhiNodesIfNeeded(ir::Control* control, const BasicBlock* block);
  void StartBlock(BasicBlock* block);

  BasicBlock* basic_block_;
  // A mapping to basic block from IR control node, which starts or ends basic
  // block.
  std::unordered_map<ir::Control*, BasicBlock*> basic_blocks_;
  const std::unique_ptr<ir::Editor> editor_;
  std::unique_ptr<VariableTracker> variable_tracker_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_BUILDER_H_
