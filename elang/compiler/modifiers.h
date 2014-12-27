// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_modifier_h)
#define INCLUDE_elang_compiler_modifier_h

#include "base/macros.h"

namespace elang {
namespace compiler {

// IGNORE_MODIFIER is a convenience macro that can be supplied as
// an argument (at any position) for a TOKEN_LIST call. It does
// nothing with tokens belonging to the respective category.
#define IGNORE_MODIFIER(name, details)

#define MODIFIER_LIST(V) \
  V(Abstract, "I") \
  V(Extern, "I") \
  V(Final, "I") \
  V(New, "M") \
  /* 'partial' modifier must be the last modifier */ \
  V(Partial, "D") \
  V(Private, "A") \
  V(Protected, "A") \
  V(Public, "A") \
  V(Static, "I") \
  V(Virtual, "I") \
  V(Volatile, "V")

//////////////////////////////////////////////////////////////////////
//
// Modifier
//
enum class Modifier {
  #define T(name, details) name,
  MODIFIER_LIST(T)
  #undef T
};

//////////////////////////////////////////////////////////////////////
//
// Modifiers
//
class Modifiers final {
  friend class ModifiersBuilder;

  private: int flags_;

  private: explicit Modifiers(int flags);
  public: Modifiers(const Modifiers& other);
  public: Modifiers();
  public: ~Modifiers();

  public: Modifiers& operator=(const Modifiers& other);

  public: int value() const { return flags_; }

  #define DEFINE_HAS(name, details) \
    bool Has ## name() const { \
      return (flags_ & (1 << static_cast<int>(Modifier::name))) != 0; \
    }
  MODIFIER_LIST(DEFINE_HAS)
  #undef DEFINE_HAS
};

static_assert(sizeof(Modifiers) == sizeof(int),
              "Instance of Modifiers should be small.");

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_modifier_h)

