// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_EDITOR_H_
#define ELANG_OPTIMIZER_EDITOR_H_

#include <vector>

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

  void SetInput(Node* node, size_t index, Node* new_value);

 private:
  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_EDITOR_H_
