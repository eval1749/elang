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

  void BindVariable(sm::Variable* variable, ir::Node* variable_value);
  void Commit();
  void EndBlockWithRet(ir::Node* data);
  ir::Node* ParameterAt(size_t index);
  void StartBlock(ir::Node* control);
  ir::Node* VariableValueOf(sm::Variable* variable) const;

 private:
  class BasicBlock;

  BasicBlock* BasicBlockOf(ir::Node* control);
  BasicBlock* NewBasicBlock(ir::Node* control, ir::Node* effect);

  BasicBlock* basic_block_;
  // A mapping from IR control node to basic block
  std::unordered_map<ir::Node*, BasicBlock*> basic_blocks_;
  const std::unique_ptr<ir::Editor> editor_;
  ir::Node* effect_;
  std::unordered_map<sm::Variable*, ir::Node*> variables_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_EDITOR_H_
