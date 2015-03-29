// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TRANSLATE_EDITOR_H_
#define ELANG_COMPILER_TRANSLATE_EDITOR_H_

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
// Editor
//
class Editor final : public ZoneOwner {
 public:
  Editor(ir::Factory* factory, ir::Function* function);
  ~Editor();

  ir::Node* control() const;

  void AssignVariable(sm::Variable* variable, ir::Node* value);
  void BindVariable(sm::Variable* variable, ir::Node* value);
  ir::Node* EndBlockWithBranch(ir::Node* condition);
  void EndBlockWithJump(ir::Node* target);
  void EndBlockWithRet(ir::Node* data);
  ir::Node* ParameterAt(size_t index);
  void StartIfBlock(ir::Node* control);
  void StartMergeBlock(ir::Node* control);
  void UnbindVariable(sm::Variable* variable);
  ir::Node* VariableValueOf(sm::Variable* variable) const;

 private:
  class BasicBlock;
  typedef std::unordered_map<sm::Variable*, ir::Node*> Variables;

  BasicBlock* BasicBlockOf(ir::Node* control);
  void EndBlock();
  BasicBlock* NewBasicBlock(ir::Node* control,
                            ir::Node* effect,
                            const Variables& variables);

  BasicBlock* basic_block_;
  // A mapping to basic block from IR control node, which starts or ends basic
  // block.
  std::unordered_map<ir::Node*, BasicBlock*> basic_blocks_;
  const std::unique_ptr<ir::Editor> editor_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_EDITOR_H_
