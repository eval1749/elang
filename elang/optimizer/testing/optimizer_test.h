// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TESTING_OPTIMIZER_TEST_H_
#define ELANG_OPTIMIZER_TESTING_OPTIMIZER_TEST_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "elang/optimizer/factory_user.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
class AtomicString;
class AtomicStringFactory;
class Zone;

namespace optimizer {
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// OptimizerTest
//
class OptimizerTest : public ::testing::Test, public FactoryUser {
 protected:
  OptimizerTest();
  ~OptimizerTest() override;

  std::string ToString(const Function* function);
  std::string ToString(const Thing* thing);

 private:
  const std::unique_ptr<AtomicStringFactory> atomic_string_factory_;
  const std::unique_ptr<Factory> factory_;

  DISALLOW_COPY_AND_ASSIGN(OptimizerTest);
};

}  // namespace testing
}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TESTING_OPTIMIZER_TEST_H_
