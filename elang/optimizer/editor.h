// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_EDITOR_H_
#define ELANG_OPTIMIZER_EDITOR_H_

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

  Node* control() const { return control_; }
  Function* function() const { return function_; }

  // Edit session
  void Commit();
  void Edit(Node* control);

  // Emit data node
  Node* EmitParameter(size_t index);

  // Emit control node
  Node* SetBranch(Node* condition);
  Node* SetJump(Node* target);
  void SetRet(Node* effect, Node* data);

  // Edit input edge
  void AppendInput(Node* node, Node* new_value);
  void ChangeInput(Node* node, size_t index, Node* new_value);
  void ReplaceAllUses(Node* new_node, Node* old_node);

  // Phi
  void SetPhiInput(Node* phi, Node* control, Node* value);

  bool Validate() const;

 private:
  Node* entry_node() const;
  Node* exit_node() const;

  Node* control_;
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_EDITOR_H_
