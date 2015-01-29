// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_
#define ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_

#include <memory>

#include "base/macros.h"
#include "elang/base/zone_owner.h"

namespace elang {

namespace hir {
class DominatorTree;
class Editor;
class Factory;
class Function;
}

namespace compiler {

class VariableUsages;

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaConverter transforms CFG to SSA form.
//
class CfgToSsaConverter final : public ZoneOwner {
 public:
  CfgToSsaConverter(hir::Factory* factory,
                    hir::Function* function,
                    const VariableUsages* usages);
  ~CfgToSsaConverter();

  void Run();

 private:
  hir::Editor* editor() { return editor_.get(); }

  std::unique_ptr<hir::Editor> editor_;
  hir::DominatorTree* const dominator_tree_;
  const VariableUsages* const variable_usages_;

  DISALLOW_COPY_AND_ASSIGN(CfgToSsaConverter);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_
