// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_TARGETS_TARGET_FEATURES_H_
#define ELANG_TARGETS_TARGET_FEATURES_H_

namespace elang {

#if ELANG_TARGET_ARCH_ARM
#define ELANG_TARGET_LITTLE_ENDIAN 1
#define ELANG_TARGET_32_BIT 1

#elif ELANG_TARGET_ARCH_ARM64
#define ELANG_TARGET_LITTLE_ENDIAN 1
#define ELANG_TARGET_64_BIT 1

#elif ELANG_TARGET_ARCH_X64
#define ELANG_TARGET_LITTLE_ENDIAN 1
#define ELANG_TARGET_64_BIT 1

#elif ELANG_TARGET_ARCH_X86
#define ELANG_TARGET_LITTLE_ENDIAN 1
#define ELANG_TARGET_32_BIT 1

#else
#error "You should define known ELANG_TARGET_ARCH_XXX."
#endif

}  // namespace elang

#endif  // ELANG_TARGETS_TARGET_FEATURES_H_
