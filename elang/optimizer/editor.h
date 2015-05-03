// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_EDITOR_H_
#define ELANG_OPTIMIZER_EDITOR_H_

#include <iosfwd>

#include "base/macros.h"
#include "elang/optimizer/error_reporter.h"
#include "elang/optimizer/factory_user.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class ELANG_OPTIMIZER_EXPORT Editor final : public ErrorReporter,
                                            public FactoryUser {
 public:
  Editor(Factory* factory, Function* function);
  ~Editor();

  Control* control() const { return control_; }
  Function* function() const { return function_; }

  // Edit session
  void Commit();
  void Edit(Control* control);

  // Data node constructor
  Data* ParameterAt(size_t index);

  // Emit control node
  Control* SetBranch(Data* condition);
  Control* SetJump(Control* target);
  Control* SetRet(Effect* effect, Data* data);

  // Edit input edge
  void AppendInput(Node* node, Node* new_value);
  void ChangeInput(Node* node, size_t index, Node* new_value);
  void RemoveControlInput(PhiOwnerNode* node, Control* control);

  // Node
  void Discard(Node* node);
  void ReplaceAllUses(Node* new_node, Node* old_node);

  // Phi
  void RemovePhiInput(EffectPhiNode* phi, Control* control);
  void RemovePhiInput(PhiNode* phi, Control* control);
  void SetPhiInput(EffectPhiNode* phi, Control* control, Effect* effect);
  void SetPhiInput(PhiNode* phi, Control* control, Data* value);

  bool Validate() const;

 private:
  EntryNode* entry_node() const;
  ExitNode* exit_node() const;

  Control* control_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const Editor& editor);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_EDITOR_H_
