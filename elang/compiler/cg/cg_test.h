// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_CG_TEST_H_
#define ELANG_COMPILER_CG_CG_TEST_H_

#include <memory>

#include "elang/compiler/testing/analyzer_test.h"

namespace elang {

namespace hir {
class Factory;
struct FactoryConfig;
}

namespace compiler {
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// CgTest is a simple harness for testing interactions with compiler.
//
class CgTest : public AnalyzerTest {
 protected:
  CgTest();
  ~CgTest() override;

  hir::Factory* factory() const { return factory_.get(); }

 private:
  const std::unique_ptr<hir::FactoryConfig> factory_config_;
  const std::unique_ptr<hir::Factory> factory_;

  DISALLOW_COPY_AND_ASSIGN(CgTest);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CG_TEST_H_
