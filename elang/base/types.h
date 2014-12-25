// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_types_h)
#define INCLUDE_elang_types_h

namespace elang {

typedef float float32_t;
typedef double float64_t;

// A simple |Maybe| type, representing a value which may or may not have a
// value
template<typename T>
struct Maybe {
  explicit Maybe(T t) : has_value(true), value(t) {}
  Maybe() : has_value(false) {}

  bool has_value;
  T value;
};

// Convenience wrapper.
template<typename T>
Maybe<T> maybe(T t) {
  return Maybe<T>(t);
}

}  // namespace elang

#endif // !defined(INCLUDE_elang_types_h)

