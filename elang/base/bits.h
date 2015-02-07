// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_BITS_H_
#define ELANG_BASE_BITS_H_

#include "base/basictypes.h"

#if COMPILER_MSVC
#include <intrin.h>
#endif

#define ELANG_HAS_BUILT_IN_POPCOUNT 0
#define ELANG_HAS_BUILT_IN_LZCNT 0

namespace elang {

// See "Hacker's Delight" for implementation of |CountXXX| funcitons.

inline int CountPopulation32(uint32_t value) {
#if ELANG_HAS_BUILT_IN_POPCOUNT && COMPILER_MSVC
  return static_cast<int>(__popcnt(value));
#else
  value = ((value >> 1) & 0x55555555) + (value & 0x55555555);
  value = ((value >> 2) & 0x33333333) + (value & 0x33333333);
  value = ((value >> 4) & 0x0f0f0f0f) + (value & 0x0f0f0f0f);
  value = ((value >> 8) & 0x00ff00ff) + (value & 0x00ff00ff);
  value = ((value >> 16) & 0x0000ffff) + (value & 0x0000ffff);
  return static_cast<int>(value);
#endif
}

inline int CountPopulation64(uint64_t value) {
#if ELANG_HAS_BUILT_IN_POPCOUNT && COMPILER_MSVC
  return static_cast<int>(__popcnt64(value));
#else
  return CountPopulation32(static_cast<uint32_t>(value >> 32)) +
         CountPopulation32(static_cast<uint32_t>(value));
#endif
}

// Returns the number of bits set in |value|.
inline int CountLeadingZeros32(uint32_t value) {
#if ELANG_HAS_BUILT_IN_LZCNT && COMPILER_MSVC
  return static_cast<int>(__lzcnt(value));
#elif COMPILER_MSVC
  unsigned long result;  // NOLINT(runtime/int)
  if (!_BitScanReverse(&result, value))
    return 32;
  return static_cast<int>(31 - result);
#else
  value = value | (value >> 1);
  value = value | (value >> 2);
  value = value | (value >> 4);
  value = value | (value >> 8);
  value = value | (value >> 16);
  return CountPopulation32(~value);
#endif
}

// Returns the number of bits set in |value|.
inline int CountLeadingZeros64(uint64_t value) {
#if ELANG_HAS_BUILT_IN_LZCNT && COMPILER_MSVC
  return static_cast<int>(__lzcnt64(value));
#elif COMPILER_MSVC
  unsigned long result;  // NOLINT(runtime/int)
  if (!_BitScanReverse64(&result, value))
    return 64;
  return static_cast<int>(63 - result);
#else
  value = value | (value >> 1);
  value = value | (value >> 2);
  value = value | (value >> 4);
  value = value | (value >> 8);
  value = value | (value >> 16);
  value = value | (value >> 32);
  return CountPopulation64(~value);
#endif
}

// Returns the number of zero bits preceding the least significant 1 bit in
// |value| if |value| is non-zero, otherwise it returns 32.
inline int CountTrailingZeros32(uint32_t value) {
#if COMPILER_MSVC
  unsigned long result;  // NOLINT(runtime/int)
  if (!_BitScanForward(&result, value))
    return 32;
  return static_cast<int>(result);
#else
  return 32 - CountPopulation32(value | ~value);
#endif
}

// Returns the number of zero bits preceding the least significant 1 bit in
// |value| if |value| is non-zero, otherwise it returns 64.
inline int CountTrailingZeros64(uint64_t value) {
#if COMPILER_MSVC
  unsigned long result;  // NOLINT(runtime/int)
  if (!_BitScanForward64(&result, value))
    return 64;
  return static_cast<int>(result);
#else
  return 64 - CountPopulation64(value | ~value);
#endif
}

// Overloaded version.
#define FOR_EACH_BASE_BITS_OPERATION(V) \
  V(CountLeadingZeros)                  \
  V(CountPopulation)                    \
  V(CountTrailingZeros)

#define V(name)                                               \
  inline int name(uint32_t value) { return name##32(value); } \
  inline int name(uint64_t value) { return name##64(value); }
FOR_EACH_BASE_BITS_OPERATION(V)
#undef V

}  // namespace elang

#endif  // ELANG_BASE_BITS_H_
