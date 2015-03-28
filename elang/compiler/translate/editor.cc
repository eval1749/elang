// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/translate/editor.h"

#include "elang/compiler/semantics/nodes.h"
#include "elang/optimizer/editor.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Editor::BasicBlock
//
class Editor::BasicBlock final : public ZoneAllocated {
 public:
  BasicBlock(ir::Node* control, ir::Node* effect);
  ~BasicBlock() = delete;

  ir::Node* control() const { return control_; }
  ir::Node* effect() const { return effect_; }
  void set_effect(ir::Node* effect);

 private:
  // Control value associated to this |BasicBlock|.
  ir::Node* const control_;

  // Effect value at the end of this |BasicBlock|.
  ir::Node* effect_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};

Editor::BasicBlock::BasicBlock(ir::Node* control, ir::Node* effect)
    : control_(control), effect_(effect) {
  DCHECK(control_->IsValidControl());
  DCHECK(effect_->IsValidEffect());
}

void Editor::BasicBlock::set_effect(ir::Node* effect) {
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK_NE(effect_, effect) << *effect;
  effect_ = effect;
}

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(ir::Factory* factory, ir::Function* function)
    : basic_block_(nullptr),
      editor_(new ir::Editor(factory, function)),
      effect_(nullptr) {
  auto const entry_node = function->entry_node();
  NewBasicBlock(editor_->NewGet(entry_node, 0), editor_->NewGet(entry_node, 1));
}

Editor::~Editor() {
  DCHECK(!basic_block_);
  DCHECK(!editor_->control());
  DCHECK(!effect_);
  DCHECK(editor_->Validate()) << editor_->errors();
}

ir::Node* Editor::control() const {
  return editor_->control();
}

Editor::BasicBlock* Editor::BasicBlockOf(ir::Node* control) {
  auto const it = basic_blocks_.find(control);
  DCHECK(it != basic_blocks_.end());
  return it->second;
}

void Editor::BindVariable(sm::Variable* variable, ir::Node* variable_value) {
  if (variable->storage() == sm::StorageClass::Void)
    return;
  DCHECK(!variables_.count(variable));
  if (variable->storage() == sm::StorageClass::ReadOnly) {
    variables_[variable] = variable_value;
    return;
  }

  // TODO(eval1749) We should introduce |sm::StorageClass::Register| and
  // use here instead of |sm::StorageClass::Local|.
  DCHECK_EQ(variable->storage(), sm::StorageClass::Local);
  variables_[variable] = variable_value;
}

void Editor::Commit() {
  DCHECK(editor_->control());
  DCHECK_EQ(basic_block_, basic_blocks_[editor_->control()]);
  editor_->Commit();
  basic_block_ = nullptr;
  effect_ = nullptr;
}

void Editor::EndBlockWithRet(ir::Node* data) {
  DCHECK(basic_block_);
  DCHECK(effect_);
  editor_->SetRet(effect_, data);
  Commit();
}

Editor::BasicBlock* Editor::NewBasicBlock(ir::Node* control, ir::Node* effect) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(effect->IsValidEffect()) << *effect;
  DCHECK(!basic_blocks_.count(control));
  auto const basic_block = new (ZoneOwner::zone()) BasicBlock(control, effect);
  basic_blocks_[control] = basic_block;
  return basic_block;
}

ir::Node* Editor::ParameterAt(size_t index) {
  return editor_->EmitParameter(index);
}

void Editor::StartBlock(ir::Node* control) {
  DCHECK(control->IsValidControl()) << *control;
  DCHECK(!basic_block_);
  DCHECK(!effect_);
  editor_->Edit(control);
  auto const it = basic_blocks_.find(control);
  DCHECK(it != basic_blocks_.end());
  basic_block_ = it->second;
  effect_ = basic_block_->effect();
}

ir::Node* Editor::VariableValueOf(sm::Variable* variable) const {
  auto const it = variables_.find(variable);
  DCHECK(it != variables_.end()) << *variable << " isn't resolved";
  DCHECK(it->second) << *variable << " has no value";
  return it->second;
}

}  // namespace compiler
}  // namespace elang
