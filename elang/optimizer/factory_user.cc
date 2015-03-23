// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/factory_user.h"

#include "elang/optimizer/factory.h"

namespace elang {
namespace optimizer {

FactoryUser::FactoryUser(Factory* factory)
    : NodeFactoryUser(factory->node_factory()),
      TypeFactoryUser(factory->type_factory()),
      factory_(factory) {
}

FactoryUser::~FactoryUser() {
}

}  // namespace optimizer
}  // namespace elang
