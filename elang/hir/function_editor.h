// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FUNCTION_EDITOR_H_
#define ELANG_HIR_FUNCTION_EDITOR_H_

#include <vector>

#include "elang/hir/hir_export.h"
#include "elang/hir/values.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// FunctionEditor
//
class ELANG_HIR_EXPORT FunctionEditor final {
 public:
  explicit FunctionEditor(Factory* factory, Function* function);
  ~FunctionEditor();

  static bool Validate(Function* function);

 private:
  Function* function_;
  std::vector<Function*> functions_;
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(FunctionEditor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FUNCTION_EDITOR_H_