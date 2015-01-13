// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TESTING_LIR_TEST_X64_H_
#define ELANG_LIR_TESTING_LIR_TEST_X64_H_

#include "elang/lir/testing/lir_test.h"

namespace elang {
namespace lir {
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// LirTest offers LIR factories.
//
class LirTestX64 : public LirTest {
 protected:
  LirTestX64() = default;

  Function* CreateFunctionSample1();

 private:
  DISALLOW_COPY_AND_ASSIGN(LirTestX64);
};

}  // namespace testing
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TESTING_LIR_TEST_X64_H_
