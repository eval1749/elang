// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TESTING_HIR_TEST_H_
#define ELANG_HIR_TESTING_HIR_TEST_H_

#include <memory>

#include "base/macros.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
class AtomicString;
class AtomicStringFactory;
class Zone;

namespace hir {
class Factory;
struct FactoryConfig;
class TypeFactory;

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// HirTest
//
class HirTest : public ::testing::Test {
 protected:
  HirTest();
  ~HirTest() override;

  Factory* factory() const { return factory_.get(); }
  TypeFactory* types();
  Zone* zone();

 private:
  std::unique_ptr<AtomicStringFactory> atomic_string_factory_;
  std::unique_ptr<FactoryConfig> factory_config_;
  std::unique_ptr<Factory> factory_;

  DISALLOW_COPY_AND_ASSIGN(HirTest);
};

}  // namespace testing
}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TESTING_HIR_TEST_H_
