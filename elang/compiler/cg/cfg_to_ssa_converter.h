// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_
#define ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_

#include <memory>

#include "base/macros.h"

namespace elang {

namespace hir {
class Factory;
class Function;
}

namespace compiler {

class VariableUsages;

//////////////////////////////////////////////////////////////////////
//
// CfgToSsaConverter transforms CFG to SSA form.
//
class CfgToSsaConverter final {
 public:
  CfgToSsaConverter(hir::Factory* factory,
                    hir::Function* function,
                    const VariableUsages* usages);
  ~CfgToSsaConverter();

  void Run();

 private:
  class Impl;

  const std::unique_ptr<Impl> impl_;

  DISALLOW_COPY_AND_ASSIGN(CfgToSsaConverter);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CFG_TO_SSA_CONVERTER_H_
