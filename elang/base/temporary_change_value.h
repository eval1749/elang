// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_TEMPORARY_CHANGE_VALUE_H_
#define ELANG_BASE_TEMPORARY_CHANGE_VALUE_H_

#include "base/basictypes.h"

namespace elang {

template <typename T>
class TemporaryChangeValue {
 public:
  TemporaryChangeValue(T& value, const T& new_value)  // NOLINT
      : old_value_(value), value_(value) {
    value_ = new_value;
  }

  ~TemporaryChangeValue() { value_ = old_value_; }

 private:
  T old_value_;
  T& value_;

  DISALLOW_COPY_AND_ASSIGN(TemporaryChangeValue);
};

}  // namespace elang

#endif  // ELANG_BASE_TEMPORARY_CHANGE_VALUE_H_
