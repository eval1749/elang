// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(elang_compiler_modifier_h)
#define elang_compiler_modifier_h

namespace elang {
namespace compiler {

// IGNORE_MODIFIER is a convenience macro that can be supplied as
// an argument (at any position) for a TOKEN_LIST call. It does
// nothing with tokens belonging to the respective category.
#define IGNORE_MODIFIER(name, details)

#define MODIFIER_LIST(V) \
  V(Abstract, "I") \
  V(Final, "I") \
  V(New, "M") \
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

}  // namespace compiler
}  // namespace elang

#endif // !defined(elang_compiler_modifier_h)

