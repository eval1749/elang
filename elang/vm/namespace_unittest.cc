// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/base/zone.h"
#include "elang/vm/factory.h"
#include "elang/vm/namespace.h"
#include "gtest/gtest.h"

namespace elang {
namespace vm {

//////////////////////////////////////////////////////////////////////
//
// NamespaceTest offers HIR factories.
//
class NamespaceTest : public ::testing::Test {
 protected:
  NamespaceTest();

  Factory* factory() { return factory_.get(); }

 private:
  std::unique_ptr<Factory> factory_;
};

NamespaceTest::NamespaceTest() : factory_(new Factory()) {
}

//////////////////////////////////////////////////////////////////////
//
// CallNamespace
//
TEST_F(NamespaceTest, CallNamespace) {
  auto const global_ns = factory()->global_namespace();
  auto const name = factory()->NewAtomicString(L"n1");
  auto const ns = factory()->NewNamespace(global_ns, name);
  EXPECT_EQ(name, ns->name());
  EXPECT_EQ(global_ns, ns->outer());
}

}  // namespace vm
}  // namespace elang
