// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_
#define ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_

#include "base/macros.h"
#include "elang/base/zone_owner.h"

namespace elang {

namespace hir {
class DominatorTree;
class Editor;
}

namespace compiler {

class VariableUsages;

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaConverter converts CFG to SSA form.
//
class CfgToSsaConverter final : public ZoneOwner {
 public:
  CfgToSsaConverter(hir::Editor* editor, const VariableUsages* usages);
  ~CfgToSsaConverter();

  // The entry point of CFG to SSA converter.
  void Run();

 private:
  hir::Editor* editor() { return editor_; }

  hir::Editor* const editor_;
  hir::DominatorTree* const dominator_tree_;
  const VariableUsages* const variable_usages_;

  DISALLOW_COPY_AND_ASSIGN(CfgToSsaConverter);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_
