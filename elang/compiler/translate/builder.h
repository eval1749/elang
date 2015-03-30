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
  ir::Control* EndBlockWithBranch(ir::Node* condition);
  void EndBlockWithJump(ir::Control* target);
  void EndBlockWithRet(ir::Node* data);
  void EndLoopBlock(ir::Node* condition,
                    ir::Control* true_target,
                    ir::Control* false_target);
  void StartIfBlock(ir::Control* control);
  ir::Control* StartLoopBlock();
  void StartMergeBlock(ir::PhiOwnerNode* control);

  // Effects
  ir::Node* Call(ir::Node* callee, ir::Node* arguments);

  // Variable management
  void AssignVariable(sm::Variable* variable, ir::Node* value);
  void BindVariable(sm::Variable* variable, ir::Node* value);
  ir::Node* ParameterAt(size_t index);
  void UnbindVariable(sm::Variable* variable);
  ir::Node* VariableValueOf(sm::Variable* variable) const;

 private:
  class BasicBlock;
  typedef std::unordered_map<sm::Variable*, ir::Node*> Variables;

  BasicBlock* BasicBlockOf(ir::Control* control);
  void EndBlock(ir::Control* control);
  BasicBlock* NewBasicBlock(ir::Control* control,
                            ir::Effect* effect,
                            const Variables& variables);
  void PopulatePhiNodesIfNeeded(ir::Control* control, const BasicBlock* block);

  BasicBlock* basic_block_;
  // A mapping to basic block from IR control node, which starts or ends basic
  // block.
  std::unordered_map<ir::Node*, BasicBlock*> basic_blocks_;
  const std::unique_ptr<ir::Editor> editor_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_BUILDER_H_
