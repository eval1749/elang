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
  V(Override, "override", "I")                       \
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
  // Helper for constructing |Modifiers| with list of modifiers.
  template <typename... Rest>
  explicit Modifiers(Modifier modifier, Rest... modifiers)
      : Modifiers((1 << static_cast<int>(modifier)) |
                  Modifiers(modifiers...).flags_) {}  // NOLINT
  Modifiers(const Modifiers& other);
  Modifiers();
  ~Modifiers();

  static Modifiers Class();
  static Modifiers Enum();
  static Modifiers Field();
  static Modifiers Method();

  Modifiers& operator=(const Modifiers& other);

  bool operator==(const Modifiers& other) const;
  bool operator!=(const Modifiers& other) const;
  Modifiers operator&(const Modifiers& other) const;
  Modifiers operator|(const Modifiers& other) const;
  Modifiers operator^(const Modifiers& other) const;

  int value() const { return flags_; }

#define V(name, string, details)                                 \
  bool Has##name() const {                                       \
    return !!(flags_ & (1 << static_cast<int>(Modifier::name))); \
  }
  FOR_EACH_MODIFIER(V)
#undef V

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
