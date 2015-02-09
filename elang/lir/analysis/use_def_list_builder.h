// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ANALYSIS_USE_DEF_LIST_BUILDER_H_
#define ELANG_LIR_ANALYSIS_USE_DEF_LIST_BUILDER_H_

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

class Function;
class Instruction;
class UseDefList;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// UseDefListBuilder
//
class ELANG_LIR_EXPORT UseDefListBuilder final {
 public:
  explicit UseDefListBuilder(Function* function);
  ~UseDefListBuilder();

  UseDefList Build();

 private:
  friend class UseDefListBuilderBuilder;

  void AddUser(UseDefList* use_def_list, Value value, Instruction* instruction);
  void Assign(UseDefList* use_def_list, Value value);

  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(UseDefListBuilder);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ANALYSIS_USE_DEF_LIST_BUILDER_H_
