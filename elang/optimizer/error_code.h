// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_ERROR_CODE_H_
#define ELANG_OPTIMIZER_ERROR_CODE_H_

namespace elang {
namespace optimizer {

#define IGNORE_OPTIMIZER_ERROR(category, subcategory, name)

#define FOR_EACH_OPTIMIZER_ERROR_CODE(E, W) \
  E(Validate, EntryNode, NoControlOutput)   \
  E(Validate, EntryNode, NoEffectOutput)    \
  E(Validate, EntryNode, NoUsers)           \
  E(Validate, EntryNode, InvalidOutput)     \
  E(Validate, ExitNode, InvalidInput)       \
  E(Validate, ExitNode, NoControlInput)     \
  E(Validate, ExitNode, NoEffectInput)      \
  E(Validate, Node, InvalidInput)

//////////////////////////////////////////////////////////////////////
//
// ErrorCode
//
enum class ErrorCode {
#define E(category, subcategory, name) category##subcategory##name,
  FOR_EACH_OPTIMIZER_ERROR_CODE(E, E)
#undef E
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_ERROR_CODE_H_
