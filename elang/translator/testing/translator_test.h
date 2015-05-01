// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_TRANSLATOR_TESTING_TRANSLATOR_TEST_H_
#define ELANG_TRANSLATOR_TESTING_TRANSLATOR_TEST_H_

#include <string>
#include <memory>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/api/pass_observer.h"
#include "elang/base/float_types.h"
#include "elang/optimizer/factory_user.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {
class Factory;
class Function;
}
namespace optimizer {
class Editor;
class Factory;
class Function;
class Node;
class Type;
}
namespace translator {
namespace ir = optimizer;
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// TranslatorTest
//
class TranslatorTest : public ::testing::Test,
                       public api::PassObserver,
                       public ir::FactoryUser {
 protected:
  TranslatorTest();
  ~TranslatorTest() override;

  lir::Factory* lir_factory() const { return lir_factory_.get(); }

  // Returns validation results as string after calling |Editor::Commit()|.
  std::string Commit(ir::Editor* editor);

  // Format |function| into human readable format.
  std::string Format(const lir::Function* function);

  // Returns formatted LIR function converted from |ir::Editor|.
  std::string Translate(const ir::Editor& editor);

  // Returns new HIR function with specified signature.
  ir::Function* NewFunction(ir::Type* return_type, ir::Type* parameters_type);

 private:
  // api::PassObserver
  void DidEndPass(api::Pass* pass) final;
  void DidStartPass(api::Pass* pass) final;

  const std::unique_ptr<ir::Factory> factory_;
  const std::unique_ptr<lir::Factory> lir_factory_;

  DISALLOW_COPY_AND_ASSIGN(TranslatorTest);
};

}  // namespace testing
}  // namespace translator
}  // namespace elang

#endif  // ELANG_TRANSLATOR_TESTING_TRANSLATOR_TEST_H_
