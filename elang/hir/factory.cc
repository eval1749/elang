// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/factory.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "elang/base/zone.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory() : InstructionFactory(this), zone_(new Zone()) {
}

Factory::~Factory() {
}

}  // namespace hir
}  // namespace elang
