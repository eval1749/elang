// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/zone_user.h"

namespace elang {

ZoneUser::ZoneUser(Zone* zone) : zone_(zone) {
}

ZoneUser::~ZoneUser() {
}

}  // namespace elang
