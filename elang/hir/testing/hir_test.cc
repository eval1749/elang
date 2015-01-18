// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/atomic_string_factory.h"
#include "elang/hir/factory.h"
#include "elang/hir/factory_config.h"
#include "elang/hir/testing/hir_test.h"

namespace elang {
namespace hir {
namespace testing {

namespace {
std::unique_ptr<FactoryConfig> NewFactoryConfig(
    AtomicStringFactory* atomic_string_factory) {
  auto config = std::make_unique<FactoryConfig>();
  config->atomic_string_factory = atomic_string_factory;
  config->string_type_name = atomic_string_factory->NewAtomicString(L"String");
  return config;
}
}  // namespace

HirTest::HirTest()
    : atomic_string_factory_(new AtomicStringFactory()),
      factory_config_(NewFactoryConfig(atomic_string_factory_.get())),
      factory_(new Factory(*factory_config_)) {
}

HirTest::~HirTest() {
}

TypeFactory* HirTest::types() {
  return factory_->types();
}

Zone* HirTest::zone() {
  return factory_->zone();
}

}  // namespace testing
}  // namespace hir
}  // namespace elang
