// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TESTING_OPTIMIZER_TEST_H_
#define ELANG_OPTIMIZER_TESTING_OPTIMIZER_TEST_H_

#include <memory>
#include <string>
#include <vector>

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

  Function* NewSampleFunction(Type* return_type, Type* parameters_type);
  Function* NewSampleFunction(Type* return_type,
                              const std::vector<Type*>& parameter_types);

  std::string ToString(const Function* function);
  std::string ToString(const Node* node);
  std::string ToString(const Type* type);

 private:
  const std::unique_ptr<AtomicStringFactory> atomic_string_factory_;
  const std::unique_ptr<Factory> factory_;
  Function* function_;

  DISALLOW_COPY_AND_ASSIGN(OptimizerTest);
};

}  // namespace testing
}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TESTING_OPTIMIZER_TEST_H_
