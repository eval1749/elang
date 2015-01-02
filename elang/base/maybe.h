// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_MAYBE_H_
#define ELANG_BASE_MAYBE_H_

namespace elang {

// A simple |Maybe| type, representing a value which may or may not have a
// value
template <typename T>
struct Maybe {
  explicit Maybe(T t) : has_value(true), value(t) {}
  Maybe() : has_value(false) {}

  bool has_value;
  T value;
};

// Convenience wrapper.
template <typename T>
Maybe<T> make_maybe(T t) {
  return Maybe<T>(t);
}

}  // namespace elang

#endif  // ELANG_BASE_MAYBE_H_
