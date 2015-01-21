// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ZONE_USER_H_
#define ELANG_BASE_ZONE_USER_H_

#include "base/macros.h"
#include "elang/base/base_export.h"

namespace elang {

class Zone;

//////////////////////////////////////////////////////////////////////
//
// ZoneUser
//
class ELANG_BASE_EXPORT ZoneUser {
 public:
  ~ZoneUser();

  Zone* zone() const { return zone_; }

 protected:
  explicit ZoneUser(Zone* zone);

 private:
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(ZoneUser);
};

}  // namespace elang

#endif  // ELANG_BASE_ZONE_USER_H_
