// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TRANSFORMS_PREPARE_PHI_INVERSION_PASS_H_
#define ELANG_LIR_TRANSFORMS_PREPARE_PHI_INVERSION_PASS_H_

#include "elang/lir/lir_export.h"
#include "elang/lir/pass.h"

namespace elang {
namespace lir {

struct Value;

//////////////////////////////////////////////////////////////////////
//
// PreparePhiInversionPass
//
class ELANG_LIR_EXPORT PreparePhiInversionPass final : public FunctionPass {
 public:
  explicit PreparePhiInversionPass(Editor* editor);
  ~PreparePhiInversionPass();

 private:
  // Pass
  base::StringPiece name() const final;

  // FunctionPass
  void RunOnFunction() final;

  DISALLOW_COPY_AND_ASSIGN(PreparePhiInversionPass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TRANSFORMS_PREPARE_PHI_INVERSION_PASS_H_
