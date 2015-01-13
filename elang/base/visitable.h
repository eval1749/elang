// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_VISITABLE_H_
#define ELANG_BASE_VISITABLE_H_

#include "base/macros.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// Derived class of |Visitable| implements visitor patter.
//
template<class Visitor>
class Visitable {
 public:
  virtual void Accept(Visitor* visitor) = 0;

 protected:
  Visitable() = default;
  ~Visitable() = default;
};

}  // namespace elang

#endif  // ELANG_BASE_VISITABLE_H_
