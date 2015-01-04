// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/api/source_code_location.h"

#include "base/logging.h"

namespace elang {
namespace api {

//////////////////////////////////////////////////////////////////////
//
// SourceCodeLocation
//
SourceCodeLocation::SourceCodeLocation(int id) : id(id) {
  DCHECK_GE(id, 0);
}

SourceCodeLocation::SourceCodeLocation() : SourceCodeLocation(0) {
}

}  // namespace api
}  // namespace elang
