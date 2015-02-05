// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_CG_TESTING_CG_TEST_H_
#define ELANG_CG_TESTING_CG_TEST_H_

#include <string>
#include <memory>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/base/float_types.h"
#include "elang/hir/factory_user.h"
#include "gtest/gtest.h"

namespace elang {
namespace hir {
class Editor;
class Factory;
class Function;
class Type;
class Value;
}
namespace lir {
class Factory;
class Function;
}
namespace cg {
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// CgTest
//
class CgTest : public ::testing::Test, public hir::FactoryUser {
 protected:
  CgTest();
  ~CgTest() override;

  lir::Factory* lir_factory() const { return lir_factory_.get(); }
  hir::Function* function() const { return function_; }

  // Format |function| into human readable format.
  std::string Format(const lir::Function* function);

  // Returns formatted LIR function converted from HIR |function|.
  std::string Generate(hir::Function* function);

  // Returns new HIR function with specified signature.
  hir::Function* NewFunction(hir::Type* return_type,
                             hir::Type* parameters_type);

  std::string Validate(hir::Editor* editor);

 private:
  const std::unique_ptr<hir::Factory> factory_;
  const std::unique_ptr<lir::Factory> lir_factory_;
  hir::Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(CgTest);
};

}  // namespace testing
}  // namespace cg
}  // namespace elang

#endif  // ELANG_CG_TESTING_CG_TEST_H_
