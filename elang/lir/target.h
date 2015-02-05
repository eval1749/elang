// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TARGET_H_
#define ELANG_LIR_TARGET_H_

#ifdef ELANG_TARGET_ARCH_X64
#include "elang/lir/target_x64.h"
#else
#error "You should define known ELANG_TARGET_ARCH_XXX."
#endif

#endif  // ELANG_LIR_TARGET_H_
