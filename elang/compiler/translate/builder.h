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
  void AppendPhiInput(ir::PhiNode* phi, ir::Control* control, ir::Data* data);
  ir::Control* EndBlockWithBranch(ir::Data* condition);
  ir::Control* EndBlockWithJump(ir::Control* target);
  void EndBlockWithRet(ir::Data* data);
  void EndLoopBlock(ir::Data* condition,
                    ir::Control* true_target,
                    ir::Control* false_target);
  ir::PhiOwnerNode* NewMergeBlock();
  void StartIfBlock(ir::Control* control);
  ir::Control* StartLoopBlock(ir::PhiOwnerNode* control);
  void StartMergeBlock(ir::PhiOwnerNode* control);
  ir::Control* StartMergeLoopBlock();

  // Effect consumer/producer
  ir::Data* Call(ir::Data* callee, ir::Node* arguments);
  ir::Data* NewLoad(ir::Data* base_pointer, ir::Data* pointer);

  // Variable management
  void AssignVariable(sm::Variable* variable, ir::Data* value);
  void BindVariable(sm::Variable* variable, ir::Data* value);
  ir::Data* ParameterAt(size_t index);
  void UnbindVariable(sm::Variable* variable);
  ir::Data* VariableValueOf(sm::Variable* variable) const;

 private:
  class BasicBlock;
  typedef std::unordered_map<sm::Variable*, ir::Data*> Variables;

  BasicBlock* BasicBlockOf(ir::Control* control);
  void EndBlock(ir::Control* control);
  BasicBlock* NewBasicBlock(ir::Control* control,
                            ir::Effect* effect,
                            const Variables& variables);
  void PopulatePhiNodesIfNeeded(ir::Control* control, const BasicBlock* block);

  BasicBlock* basic_block_;
  // A mapping to basic block from IR control node, which starts or ends basic
  // block.
  std::unordered_map<ir::Control*, BasicBlock*> basic_blocks_;
  const std::unique_ptr<ir::Editor> editor_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_BUILDER_H_
