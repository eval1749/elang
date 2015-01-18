// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/cg_test.h"

#include "base/macros.h"
#include "elang/compiler/compilation_session.h"
#include "elang/hir/factory.h"
#include "elang/hir/factory_config.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {
std::unique_ptr<hir::FactoryConfig> NewFactoryConfig(
    CompilationSession* session) {
  auto config = std::make_unique<hir::FactoryConfig>();
  config->atomic_string_factory = session->atomic_string_factory();
  config->string_type_name = session->NewAtomicString(L"System.String");
  return config;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CgTest
//
CgTest::CgTest()
    : factory_config_(NewFactoryConfig(session())),
      factory_(new hir::Factory(*factory_config_)) {
}

CgTest::~CgTest() {
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
