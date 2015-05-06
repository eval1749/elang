// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_DUMP_FUNCTION_PASS_H_
#define ELANG_LIR_DUMP_FUNCTION_PASS_H_

#include "elang/lir/pass.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// DumpFunctionPass
//
class DumpFunctionPass : public FunctionPass {
 public:
  DumpFunctionPass(base::StringPiece name, Editor* editor);
  ~DumpFunctionPass() final;

 private:
  // FunctionPass
  void RunOnFunction() final;

  DISALLOW_COPY_AND_ASSIGN(DumpFunctionPass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_DUMP_FUNCTION_PASS_H_
