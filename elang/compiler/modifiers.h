// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_MODIFIERS_H_
#define ELANG_COMPILER_MODIFIERS_H_

#include <ostream>
#include <string>

#include "base/macros.h"

namespace elang {
namespace compiler {

// IGNORE_MODIFIER is a convenience macro that can be supplied as
// an argument (at any position) for a FOR_EACH_TOKEN call. It does
// nothing with tokens belonging to the respective category.
#define IGNORE_MODIFIER(name, string, details)

#define FOR_EACH_MODIFIER(V)                         \
  V(Abstract, "abstract", "I")                       \
  V(Extern, "extern", "I")                           \
  V(Final, "final", "I")                             \
  V(New, "new", "M")                                 \
  /* 'partial' modifier must be the last modifier */ \
  V(Partial, "partial", "D")                         \
  V(Private, "private", "A")                         \
  V(Protected, "protected", "A")                     \
  V(Public, "public", "A")                           \
  V(Static, "static", "I")                           \
  V(Virtual, "virtual", "I")                         \
  V(Volatile, "volatile", "V")

//////////////////////////////////////////////////////////////////////
//
// Modifier
//
enum class Modifier {
#define T(name, string, details) name,
  FOR_EACH_MODIFIER(T)
#undef T
};

//////////////////////////////////////////////////////////////////////
//
// Modifiers
//
class Modifiers final {
  friend class ModifiersBuilder;

 public:
  Modifiers(const Modifiers& other);
  Modifiers();
  ~Modifiers();

  Modifiers& operator=(const Modifiers& other);

  int value() const { return flags_; }

#define DEFINE_HAS(name, string, details)                           \
  bool Has##name() const {                                          \
    return (flags_ & (1 << static_cast<int>(Modifier::name))) != 0; \
  }
  FOR_EACH_MODIFIER(DEFINE_HAS)
#undef DEFINE_HAS

 private:
  explicit Modifiers(int flags);

  int flags_;
};

static_assert(sizeof(Modifiers) == sizeof(int),
              "Instance of Modifiers should be small.");

std::ostream& operator<<(std::ostream& ostream, const Modifiers& modifiers);

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_MODIFIERS_H_
