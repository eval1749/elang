// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_API_SOURCE_CODE_LOCATION_H_
#define ELANG_API_SOURCE_CODE_LOCATION_H_

#include "base/macros.h"

namespace elang {
namespace api {

//////////////////////////////////////////////////////////////////////
//
// SourceCodeLocation
//
struct SourceCodeLocation {
  int id;

  explicit SourceCodeLocation(int id);
  SourceCodeLocation();
};

static_assert(sizeof(SourceCodeLocation) == sizeof(int),
              "SourceCodeLocation should be a POD.");

}  // namespace api
}  // namespace elang

#endif  // ELANG_API_SOURCE_CODE_LOCATION_H_
