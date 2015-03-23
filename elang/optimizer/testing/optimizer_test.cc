// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/base/atomic_string_factory.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/factory_config.h"
#include "elang/optimizer/testing/optimizer_test.h"
#include "elang/optimizer/thing.h"

namespace elang {
namespace optimizer {
namespace testing {

namespace {
Factory* NewFactory() {
  FactoryConfig config;
  auto const atomic_string_factory = new AtomicStringFactory();
  config.atomic_string_factory = atomic_string_factory;
  config.string_type_name =
      atomic_string_factory->NewAtomicString(L"System.String");
  return new Factory(config);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// OptimizerTest
//
OptimizerTest::OptimizerTest()
    : FactoryUser(NewFactory()),
      atomic_string_factory_(factory()->config().atomic_string_factory),
      factory_(factory()) {
}

OptimizerTest::~OptimizerTest() {
}

std::string OptimizerTest::ToString(const Thing* thing) {
  std::stringstream ostream;
  ostream << *thing;
  return ostream.str();
}

}  // namespace testing
}  // namespace optimizer
}  // namespace elang
