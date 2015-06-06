// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/optimizer/testing/optimizer_test.h"

#include "base/logging.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/factory_config.h"
#include "elang/optimizer/formatters/text_formatter.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/validator.h"

namespace elang {
namespace optimizer {
namespace testing {

namespace {

Factory* NewFactory(api::PassController* pass_controller) {
  FactoryConfig config;
  auto const atomic_string_factory = new AtomicStringFactory();
  config.atomic_string_factory = atomic_string_factory;
  config.string_type_name =
      atomic_string_factory->NewAtomicString(L"System.String");
  return new Factory(pass_controller, config);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// OptimizerTest
//
OptimizerTest::OptimizerTest()
    : FactoryUser(NewFactory(this)),
      atomic_string_factory_(factory()->config().atomic_string_factory),
      factory_(factory()),
      function_(nullptr) {
}

OptimizerTest::~OptimizerTest() {
}

Function* OptimizerTest::NewSampleFunction(Type* return_type,
                                           Type* parameters_type) {
  DCHECK(!function_);
  function_ = NewFunction(NewFunctionType(return_type, parameters_type));
  return function_;
}

Function* OptimizerTest::NewSampleFunction(
    Type* return_type,
    const std::vector<Type*>& parameter_types) {
  return NewSampleFunction(return_type, NewTupleType(parameter_types));
}

std::string OptimizerTest::ToString(const Function* function) {
  std::ostringstream ostream;
  Validator validator(factory(), function);
  if (validator.Validate())
    ostream << AsReversePostOrder(function);
  else
    ostream << factory()->errors();
  return ostream.str();
}

std::string OptimizerTest::ToString(const Node* node) {
  if (function_) {
    Validator validator(factory(), function_);
    if (!validator.Validate(node)) {
      std::ostringstream ostream;
      ostream << factory()->errors();
      return ostream.str();
    }
  }
  std::ostringstream ostream;
  ostream << *node;
  return ostream.str();
}

std::string OptimizerTest::ToString(const Type* type) {
  std::ostringstream ostream;
  ostream << *type;
  return ostream.str();
}

}  // namespace testing
}  // namespace optimizer
}  // namespace elang
