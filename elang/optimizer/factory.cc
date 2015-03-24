// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/optimizer/factory.h"

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/base/zone.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/node_factory.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory(const FactoryConfig& config)
    : NodeFactoryUser(new NodeFactory(new TypeFactory(config))),
      TypeFactoryUser(node_factory()->type_factory()),
      atomic_string_factory_(config.atomic_string_factory),
      config_(config),
      node_factory_(node_factory()),
      type_factory_(type_factory()) {
}

Factory::~Factory() {
}

AtomicString* Factory::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

}  // namespace optimizer
}  // namespace elang
