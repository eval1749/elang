// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_EDITOR_H_
#define ELANG_OPTIMIZER_EDITOR_H_

#include <stack>

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

  // Emit data node
  Node* EmitParameter(size_t index);

  // Emit terminator node
  void EndIf();
  void EndWithRet(Node* data);

  // Emit control node
  void StartElse();
  void StartIf(Node* data);
  void StartThen();

  // Edit input edges
  void SetControl(Node* node, size_t index, Node* new_control);
  void SetEffect(Node* node, size_t index, Node* new_control);
  void SetInput(Node* node, size_t index, Node* new_value);

 private:
  Node* entry_node() const;
  Node* exit_node() const;

  Node* PopControl();

  Function* const function_;
  std::stack<Node*> control_stack_;
  Node* data_node_;
  Node* effect_node_;
  std::stack<Node*> terminator_stack_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_EDITOR_H_
